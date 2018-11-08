#pragma once

#include "parser/pipes.hpp"
#include "bytecode/interpreter.hpp"

struct Inst_Jump;

#define INST(node, name, ...) this->add_instruction(node, new Inst_##name(__VA_ARGS__));

#define PUSH_L(var_name, value) auto var_name = this->is_left_value; this->is_left_value = value
#define POP_L(var_name) this->is_left_value = var_name

struct Bytecode_Generator : Pipe {
	Ast_Declaration* reg_declarations[INTERP_REGISTER_COUNT] = {};
	bool reserved[INTERP_REGISTER_COUNT] = {};
	size_t data_offset = 0;

	vector<Instruction*>* bytecode = NULL;
	bool is_left_value = false;

	vector<Inst_Jump*> pending_breaks;

	// for metrics
	size_t instruction_count = 0;

	PIPE_NAME(Bytecode_Generator)

	void add_instruction (Ast* node, Instruction* intruction) {
	    intruction->location = node->location;
	    this->bytecode->push_back(intruction);
		this->instruction_count += 1;
	}

	void handle (Ast_Note**) { /* Notes don't produce any bytecode by themselves */ }

    void handle (Ast_Statement** stm_ptr) {
		this->is_left_value = false;
		Pipe::handle(stm_ptr);
		//this->clear_registers();
	}

	void handle (Ast_Block** block_ptr) {
		for (auto &stm : (*block_ptr)->list) {
			this->handle(&stm);
		}
	}

	void handle (Ast_Return** ret_ptr) {
		auto ret = (*ret_ptr);

	    Pipe::handle(ret_ptr);

		if (ret->exp) {
			auto bytecode_type = bytecode_get_type(ret->exp->inferred_type);
			INST(ret, Return, ret->exp->reg, bytecode_type);
		} else INST(ret, Return);
	}

	void handle (Ast_If** _if_ptr) {
		auto _if = (*_if_ptr);

		Pipe::handle(&_if->condition);

		auto inst = new Inst_Jump_If_False(_if->condition->reg);
		this->add_instruction(_if, inst);

		auto index1 = this->bytecode->size();
		Pipe::handle(&_if->then_statement);
		inst->offset = this->bytecode->size() - index1;
		if (_if->else_statement) {
			inst->offset += 1;
			auto inst2 = new Inst_Jump();
			this->add_instruction(_if, inst2);

			index1 = this->bytecode->size();
			Pipe::handle(&_if->else_statement);
			inst2->offset = this->bytecode->size() - index1;
		}
	}

	void handle (Ast_While** _while_ptr) {
		auto _while = (*_while_ptr);

		auto index1 = this->bytecode->size();

		Pipe::handle(&_while->condition);

		auto jmp1 = new Inst_Jump_If_False(_while->condition->reg);
		this->add_instruction(_while, jmp1);
		auto index2 = this->bytecode->size();

		Pipe::handle(&_while->statement);

		auto jmp2 = new Inst_Jump();
		this->add_instruction(_while, jmp2);
		jmp2->offset = index1 - this->bytecode->size();
		jmp1->offset = this->bytecode->size() - index2;

		this->update_pending_breaks();
	}

	void handle (Ast_Break** _break_ptr) {
		auto jump = new Inst_Jump();
		this->add_instruction((*_break_ptr), jump);
		this->pending_breaks.push_back(jump);
		jump->offset = this->bytecode->size();
	}

    void fill (Ast_Function* func) {
		if (func->scope && func->bytecode.size() == 0) {
	        auto _tmp = this->bytecode;
	        this->bytecode = &func->bytecode;

			auto free_reg = (uint8_t) func->type->arg_decls.size();
	        for (uint8_t i = 0; i < func->type->arg_decls.size(); i++) {
	            auto decl = func->type->arg_decls[i];
				if (decl->is_spilled) {
					auto decl_type = static_cast<Ast_Type_Instance*>(decl->type);

					INST(decl, Stack_Allocate, decl_type->byte_size);
					decl->bytecode_data_offset = this->data_offset;
					this->data_offset += decl_type->byte_size;

					INST(decl, Stack_Offset, free_reg, decl->bytecode_data_offset);
					if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
						INST(decl, Copy_Memory, free_reg, i, decl_type->byte_size);
					} else {
						INST(decl, Store, free_reg, i, decl_type->byte_size);
					}
				}
	        }

			this->handle(&func->scope);

			if (func->type->ret_type == Compiler::instance->types->type_def_void) {
				if (this->bytecode->size() == 0) {
					this->add_instruction(func->scope, new Inst_Return());
				} else {
					auto last_inst = this->bytecode->back();
					if (last_inst->bytecode != BYTECODE_RETURN) {
						this->add_instruction(func->scope, new Inst_Return());
					}
				}
			}

	        this->bytecode = _tmp;
		}
	}

    void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

	    if (decl->is_constant()) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
	            auto func = static_cast<Ast_Function*>(decl->expression);
	            if (!func->foreign_module_name) {
	    			auto _tmp = this->data_offset;
	    			this->fill(func);
	                this->data_offset = _tmp;
	            }
			}
	    } else {
			auto decl_type = static_cast<Ast_Type_Instance*>(decl->type);
			if (decl->is_global()) {
				decl->bytecode_data_offset = Compiler::instance->interp->globals->add(decl_type->byte_size);
			} else {
				if (decl->expression) Pipe::handle(&decl->expression);

				if (decl->is_spilled) {
					INST(decl, Stack_Allocate, decl_type->byte_size);
					decl->bytecode_data_offset = this->data_offset;
					this->data_offset += decl_type->byte_size;

					if (decl->expression) {
						uint8_t free_reg = decl->expression->reg + 1;
						INST(decl, Stack_Offset, free_reg, decl->bytecode_data_offset);
						if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
							INST(decl, Copy_Memory, free_reg, decl->expression->reg, decl_type->byte_size);
						} else {
							INST(decl, Store, free_reg, decl->expression->reg, decl_type->byte_size);
						}
					}
				}
			}
		}
	}

	void handle (Ast_Expression** exp_ptr) {
		if (this->is_left_value != false) {
			auto tmp = this->is_left_value;
			this->is_left_value = false;
			Pipe::handle(exp_ptr);
			this->is_left_value = tmp;
		} else Pipe::handle(exp_ptr);
	}

	void handle_left (Ast_Expression** exp_ptr) {
		if (this->is_left_value != true) {
			auto tmp = this->is_left_value;
			this->is_left_value = true;
			Pipe::handle(exp_ptr);
			this->is_left_value = tmp;
		} else Pipe::handle(exp_ptr);
	}

	void handle (Ast_Cast** cast_ptr) {
		auto cast = (*cast_ptr);

		Pipe::handle(&cast->value);

		auto type_from = bytecode_get_type(cast->value->inferred_type);
		auto type_to = bytecode_get_type(cast->inferred_type);
		INST(cast, Cast, cast->reg, cast->value->reg, type_from, type_to);
	}

	uint8_t get_bytecode_from_unop (Ast_Unary_Type unop) {
		switch (unop) {
			case AST_UNARY_NEGATE:  return BYTECODE_ARITHMETIC_NEGATE;
			case AST_UNARY_NOT:     return BYTECODE_LOGICAL_NEGATE;
			default: 				abort();
		}
	}

	void handle (Ast_Unary** unop_ptr) {
		auto unop = (*unop_ptr);

		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
				Pipe::handle(&unop->exp);
				auto unop_type = get_bytecode_from_unop(unop->unary_op);
				auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
	            INST(unop, Unary, unop_type, unop->reg, unop->exp->reg, bytecode_type);
				break;
			}
			case AST_UNARY_NEGATE: {
				Pipe::handle(&unop->exp);
				auto unop_type = get_bytecode_from_unop(unop->unary_op);
				auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
				auto result_type = bytecode_unsigned_to_signed(bytecode_type);
				if (bytecode_type != result_type) {
					INST(unop, Cast, unop->exp->reg, bytecode_type, result_type);
				}
	            INST(unop, Unary, unop_type, unop->reg, unop->exp->reg, result_type);
				break;
			}
			case AST_UNARY_REFERENCE: {
				this->handle_left(&unop->exp);
				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				Pipe::handle(&unop->exp);
				if (!this->is_left_value) {
					INST(unop, Load, unop->reg, unop->exp->reg, unop->inferred_type->byte_size);
				}
	            break;
	        }
			default: abort();
		}
	}

	uint8_t get_bytecode_from_binop (Ast_Binary_Type binop) {
		switch (binop) {
			case AST_BINARY_LOGICAL_AND: 			return BYTECODE_LOGICAL_AND;
			case AST_BINARY_LOGICAL_OR: 			return BYTECODE_LOGICAL_OR;

			case AST_BINARY_ADD: 					return BYTECODE_ADD;
			case AST_BINARY_SUB: 					return BYTECODE_SUB;
			case AST_BINARY_MUL: 					return BYTECODE_MUL;
			case AST_BINARY_DIV: 					return BYTECODE_DIV;
			case AST_BINARY_REM: 					return BYTECODE_REM;

			case AST_BINARY_BITWISE_AND: 			return BYTECODE_BITWISE_AND;
			case AST_BINARY_BITWISE_OR: 			return BYTECODE_BITWISE_OR;
			case AST_BINARY_BITWISE_XOR: 			return BYTECODE_BITWISE_XOR;
			case AST_BINARY_BITWISE_RIGHT_SHIFT: 	return BYTECODE_BITWISE_RIGHT_SHIFT;
			case AST_BINARY_BITWISE_LEFT_SHIFT: 	return BYTECODE_BITWISE_LEFT_SHIFT;

			case AST_BINARY_EQ:						return BYTECODE_EQ;
			case AST_BINARY_NEQ:					return BYTECODE_NEQ;
			case AST_BINARY_LT:						return BYTECODE_LT;
			case AST_BINARY_LTE:					return BYTECODE_LTE;
			case AST_BINARY_GT:						return BYTECODE_GT;
			case AST_BINARY_GTE:					return BYTECODE_GTE;

			default: 								abort();
		}
	}

	void handle (Ast_Binary** binop_ptr) {
		auto binop = (*binop_ptr);

		switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
	        	this->handle_left(&binop->lhs);

				if (binop->reg != binop->lhs->reg) {
					INST(binop, Copy, binop->reg, binop->lhs->reg);
				}

	            // We follow the pointer until we hit a non-pointer type
				if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_POINTER) {
					auto ptr_type = static_cast<Ast_Pointer_Type*>(binop->lhs->inferred_type);
					auto base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
					while (base_type->typedef_type == AST_TYPEDEF_POINTER) {
						INST(binop, Load, binop->reg, binop->reg, ptr_type->byte_size);
						ptr_type = static_cast<Ast_Pointer_Type*>(base_type);
						base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
					}
				}

	            auto ident = static_cast<Ast_Ident*>(binop->rhs);
	            auto decl = ident->declaration;
				if (decl->attribute_byte_offset != 0) {
					INST(binop, Add_Const, binop->lhs->reg, decl->attribute_byte_offset);
				}

	            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                INST(binop, Load, binop->reg, binop->lhs->reg, binop->inferred_type->byte_size);
	            }

				break;
			}
			case AST_BINARY_SUBSCRIPT: {
				if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
		        	this->handle_left(&binop->lhs);

					Pipe::handle(&binop->rhs);

					auto array_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
					auto array_base_type = static_cast<Ast_Type_Instance*>(array_type->base);
		            auto element_size = array_base_type->byte_size;

		            if (element_size > 1) INST(binop, Mul_Const, binop->rhs->reg, element_size);

		            INST(binop, Binary, BYTECODE_ADD, binop->reg, binop->lhs->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

		            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    INST(binop, Load, binop->reg, binop->reg, binop->inferred_type->byte_size);
	                }
				} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
					auto struct_type = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
					if (struct_type->is_slice) {
						auto data_decl = struct_type->find_attribute("data");
						if (data_decl) {
							auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
							auto array_base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
				            auto element_size = array_base_type->byte_size;

				        	this->handle_left(&binop->lhs);

							INST(binop, Add_Const, binop->reg, binop->lhs->reg, data_decl->attribute_byte_offset);
							INST(binop, Load, binop->reg, binop->reg, ptr_type->byte_size);
							Pipe::handle(&binop->rhs);

				            if (element_size > 1) INST(binop, Mul_Const, binop->reg, element_size);

				            INST(binop, Binary, BYTECODE_ADD, binop->reg, binop->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

			                if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			                    INST(binop, Load, binop->reg, binop->reg, binop->inferred_type->byte_size);
			                }
						} else abort();
					} else ERROR_STOP(binop->lhs, "Struct is not a slice");
				}
				break;
			}
			case AST_BINARY_ASSIGN: {
	            auto size = binop->rhs->inferred_type->byte_size;
				Pipe::handle(&binop->rhs);

	        	this->handle_left(&binop->lhs);

				auto ident = static_cast<Ast_Ident*>(binop->lhs);
				if (binop->lhs->exp_type == AST_EXPRESSION_IDENT && !ident->declaration->is_spilled) {
					INST(binop, Copy, ident->reg, binop->rhs->reg);
				} else {
					if (size > INTERP_REGISTER_SIZE) {
						INST(binop, Copy_Memory, binop->lhs->reg, binop->rhs->reg, size);
					} else {
						INST(binop, Store, binop->lhs->reg, binop->rhs->reg, size);
					}
				}

				break;
			}
			default: {
				Pipe::handle(&binop->lhs);
				Pipe::handle(&binop->rhs);

				auto binop_type = get_bytecode_from_binop(binop->binary_op);
	            auto bytecode_type = bytecode_get_type(binop->lhs->inferred_type);
	            INST(binop, Binary, binop_type, binop->reg, binop->lhs->reg, binop->rhs->reg, bytecode_type);

				break;
			}
		}
	}

	void handle (Ast_Literal** lit_ptr) {
		auto lit = (*lit_ptr);

		switch (lit->literal_type) {
			case AST_LITERAL_SIGNED_INT:
			case AST_LITERAL_UNSIGNED_INT: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            INST(lit, Set, lit->reg, bytecode_type, &lit->int_value);
				break;
			}
			case AST_LITERAL_DECIMAL: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            if (bytecode_type == BYTECODE_TYPE_F32) {
	                auto _tmp = (float) lit->decimal_value;
	                INST(lit, Set, lit->reg, bytecode_type, &_tmp);
	            } else {
	                INST(lit, Set, lit->reg, bytecode_type, &lit->decimal_value);
	            }
	            break;
	        }
			case AST_LITERAL_STRING: {
				lit->data_offset = Compiler::instance->interp->constants->add(lit->string_value);
	            INST(lit, Constant_Offset, lit->reg, lit->data_offset);
				break;
			}
			default: ERROR_STOP(lit, "Literal type to bytecode conversion not supported!");
		}
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (ident->declaration->is_spilled) {
			if (ident->declaration->is_global()) {
				INST(ident, Global_Offset, ident->reg, ident->declaration->bytecode_data_offset);
			} else {
				INST(ident, Stack_Offset, ident->reg, ident->declaration->bytecode_data_offset);
			}

			if (!this->is_left_value && ident->inferred_type->can_be_in_register(INTERP_REGISTER_SIZE)) {
				INST(ident, Load, ident->reg, ident->reg, ident->inferred_type->byte_size);
			}
		}
	}

    void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

		if (func->is_native()) {
			INST(func, Set, func->reg, BYTECODE_TYPE_POINTER, &func->foreign_function_pointer);
		} else {
			INST(func, Set, func->reg, BYTECODE_TYPE_POINTER, &func);
		}
	}

	void handle (Ast_Function_Call** call_ptr) {
		auto call = (*call_ptr);

		for (auto &exp : call->arguments) {
			Pipe::handle(&exp);
		}

		INST(call, Call_Setup, DC_CALL_C_X64_WIN64, (uint8_t) call->arguments.size());

		auto bytecode_type = BYTECODE_TYPE_VOID;
		for (uint8_t i = 0; i < call->arguments.size(); i++) {
			auto exp = call->arguments[i];

			if (exp->inferred_type->byte_size > INTERP_REGISTER_SIZE) {
	            bytecode_type = BYTECODE_TYPE_POINTER;
	        } else {
				bytecode_type = bytecode_get_type(exp->inferred_type);
			}

			INST(call, Call_Param, i, exp->reg, bytecode_type);
		}

		auto ret_type = bytecode_get_type(call->inferred_type);
		if (call->func->exp_type == AST_EXPRESSION_FUNCTION) {
			auto func = static_cast<Ast_Function*>(call->func);
			if (func->is_native()) {
				INST(call, Call_Const, (uint64_t) func->foreign_function_pointer, call->reg, ret_type);
			} else {
				INST(call, Call_Const, (uint64_t) func, call->reg, ret_type);
			}
		} else {
			Pipe::handle(&call->func);
		    INST(call, Call, call->reg, call->func->reg, ret_type);
		}
	}

	// @Incomplete @Fixme this will break for nested loops: inner loop will "resolve"
	// the break already found in the outter loop. Break (& continue) STMs should be directly
	// associated with the loop statement itself, ideally prior to this pipe.
	void update_pending_breaks () {
		while (!this->pending_breaks.empty()) {
			auto jump = this->pending_breaks.back();
			jump->offset = this->bytecode->size() - jump->offset;
			this->pending_breaks.pop_back();
		}
	}

	void print_pipe_metrics () {
		PRINT_METRIC("Instructions created:  %zd", this->instruction_count);
	}
};
