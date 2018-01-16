#pragma once

#include "parser/pipes.hpp"
#include "bytecode/interpreter.hpp"

struct Inst_Jump;

#define INST(node, name, ...) this->add_instruction(node, new Inst_##name(__VA_ARGS__));

#define ERROR(node, ...) report_error_stop(&node->location, __VA_ARGS__)

#define PUSH_L(var_name, value) auto var_name = this->is_left_value; this->is_left_value = value
#define POP_L(var_name) this->is_left_value = var_name

struct Bytecode_Generator : Pipe {
	Ast_Declaration* reg_declarations[INTERP_REGISTER_COUNT] = {};
	bool reserved[INTERP_REGISTER_COUNT] = {};
	size_t data_offset = 0;

	vector<Instruction*>* bytecode = NULL;
	bool is_left_value = false;
	uint8_t reg = 0;

	vector<Inst_Jump*> pending_breaks;

	// for metrics
	size_t instruction_count = 0;

	PIPE_NAME(Bytecode_Generator)

	uint8_t get_free_reg (int8_t preferred = -1) {
		if (preferred == -1) {
			for (uint8_t i = 0; i < INTERP_REGISTER_COUNT; i++) {
				if (!this->reserved[i]) return i;
			}
		} else {
			if (!this->is_occupied(preferred)) return preferred;
			else {
				for (uint8_t i = 0; i < INTERP_REGISTER_COUNT; i++) {
					if (!this->reserved[i]) return i;
				}
			}
		}
		// @Incomplete we should spill memory to create a free register
		abort();
	}

	uint8_t push_expression (Ast_Expression* exp) {
		auto _reg = get_free_reg();
		this->reserved[_reg] = true;
		exp->reg = _reg;
		return _reg;
	}

	void push_declaration (Ast_Declaration* decl, uint8_t _reg) {
		this->reg_declarations[_reg] = decl;
	}

	void clear_registers () {
		/*auto free_reg = get_free_reg();
		for (uint8_t i = 0; i < INTERP_REGISTER_COUNT; i++) {
			auto decl = this->reg_declarations[i];
			if (decl) {
				auto decl_type = static_cast<Ast_Type_Instance*>(decl->type);
				INST(decl, Stack_Offset, free_reg, decl->data_offset);
				if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
					INST(decl, Copy_Memory, free_reg, i, decl_type->byte_size);
				} else {
					INST(decl, Store, free_reg, i, decl_type->byte_size);
				}
			}
		}*/
		memset(this->reg_declarations, 0, INTERP_REGISTER_COUNT * sizeof(Ast_Declaration*));
		memset(this->reserved, 0, INTERP_REGISTER_COUNT * sizeof(bool));
	}

	bool is_occupied (uint8_t reg_index) {
		return this->reg_declarations[reg_index];
	}

	void add_instruction (Ast* node, Instruction* intruction) {
	    intruction->location = node->location;
	    this->bytecode->push_back(intruction);
		this->instruction_count += 1;
	}

	void handle (Ast_Note**) { /* Notes don't produce any bytecode by themselves */ }

    void handle (Ast_Statement** stm_ptr) {
		this->reg = 0;
		this->is_left_value = false;
		Pipe::handle(stm_ptr);
		this->clear_registers();
	}

	void handle (Ast_Block** block_ptr) {
		for (auto &stm : (*block_ptr)->list) {
			this->handle(&stm);
		}
	}

	void handle (Ast_Return** ret_ptr) {
	    Pipe::handle(ret_ptr);
	    this->add_instruction((*ret_ptr), new Inst_Return());
	}

	void handle (Ast_If** _if_ptr) {
		auto _if = (*_if_ptr);

		Pipe::handle(&_if->condition);

		auto inst = new Inst_Jump_If_False(0);
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

		auto jmp1 = new Inst_Jump_If_False(0);
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

	        auto free_reg = (uint8_t) func->arg_decls.size();
	        for (uint8_t i = 0; i < func->arg_decls.size(); i++) {
	            auto decl = func->arg_decls[i];

	            auto decl_type = static_cast<Ast_Type_Instance*>(decl->type);
	            decl->data_offset = this->data_offset;
	            this->data_offset += decl_type->byte_size;

	            INST(decl, Stack_Allocate, decl_type->byte_size);
	            INST(decl, Stack_Offset, free_reg, decl->data_offset);
	            if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
	                INST(decl, Copy_Memory, free_reg, i, decl_type->byte_size);
	            } else {
	                INST(decl, Store, free_reg, i, decl_type->byte_size);
	            }
	        }

			this->handle(&func->scope);

			if (func->ret_type == g_compiler->type_def_void) {
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

	    if (decl->decl_flags & AST_DECL_FLAG_CONSTANT) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
	            auto func = static_cast<Ast_Function*>(decl->expression);
	            if (!func->foreign_module_name) {
	    			auto _tmp = this->data_offset;
	    			this->fill(func);
	                this->data_offset = _tmp;
	            }
			}
	    } else {
			auto ty_decl = static_cast<Ast_Type_Instance*>(decl->type);
			if (decl->is_global()) {
				decl->data_offset = g_compiler->interp->globals->add(ty_decl->byte_size);
			} else {
	            INST(decl, Stack_Allocate, ty_decl->byte_size);

				decl->data_offset = this->data_offset;
				this->data_offset += ty_decl->byte_size;
				if (decl->expression) {
					PUSH_L(tmp, false);
					Pipe::handle(&decl->expression);
					POP_L(tmp);

					this->push_declaration(decl, decl->expression->reg);

	                INST(decl, Stack_Offset, 1, decl->data_offset);
					if (ty_decl->byte_size > INTERP_REGISTER_SIZE) {
		                INST(decl, Copy_Memory, 1, 0, ty_decl->byte_size);
		            } else {
						INST(decl, Store, 1, 0, ty_decl->byte_size);
		            }
				}
			}
		}
	}

	void handle (Ast_Cast** cast_ptr) {
		auto cast = (*cast_ptr);

		PUSH_L(tmp, false);
		Pipe::handle(&cast->value);
		POP_L(tmp);

		cast->reg = this->get_free_reg(cast->value->reg);

		auto type_from = bytecode_get_type(cast->value->inferred_type);
		auto type_to = bytecode_get_type(cast->inferred_type);
		INST(cast, Cast, cast->reg, type_from, type_to);
	}

	uint8_t get_bytecode_from_unop (Ast_Unary_Type unop) {
		switch (unop) {
			case AST_UNARY_NEGATE:  return BYTECODE_ARITHMETIC_NEGATE;
			case AST_UNARY_NOT:     return BYTECODE_LOGICAL_NEGATE;
			default: 				return BYTECODE_NOOP;
		}
	}

	void handle (Ast_Unary** unop_ptr) {
		auto unop = (*unop_ptr);

		PUSH_L(tmp, unop->unary_op == AST_UNARY_REFERENCE);
		Pipe::handle(&unop->exp);
		POP_L(tmp);

		auto unop_type = get_bytecode_from_unop(unop->unary_op);
		auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);

		unop->reg = this->get_free_reg(unop->exp->reg);

		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
	            INST(unop, Unary, unop_type, unop->exp->reg, bytecode_type);
				break;
			}
			case AST_UNARY_NEGATE: {
				auto result_type = bytecode_unsigned_to_signed(bytecode_type);
				if (bytecode_type != result_type) {
					INST(unop, Cast, unop->exp->reg, bytecode_type, result_type);
				}
	            INST(unop, Unary, unop_type, unop->exp->reg, result_type);
				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				if (!this->is_left_value) {
					INST(unop, Load, this->reg - 1, this->reg - 1, unop->inferred_type->byte_size);
				}
	            break;
	        }
			default: break;
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

			default: 								return BYTECODE_NOOP;
		}
	}

	void handle (Ast_Binary** binop_ptr) {
		auto binop = (*binop_ptr);

		switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
				PUSH_L(tmp, true);
	        	Pipe::handle(&binop->lhs);
				POP_L(tmp);

	            // We follow the pointer until we hit a non-pointer type
	            auto type_def = binop->lhs->inferred_type;
	            while (type_def->typedef_type == AST_TYPEDEF_POINTER) {
	                auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
	                type_def = static_cast<Ast_Type_Instance*>(ptr_type->base);
	                INST(binop, Load, reg - 1, reg - 1, type_def->byte_size);
	            }

	            auto ident = static_cast<Ast_Ident*>(binop->rhs);
	            auto decl = ident->declaration;
				if (decl->attribute_byte_offset != 0) {
					INST(binop, Add_Const, reg - 1, decl->attribute_byte_offset);
				}

	            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                INST(binop, Load, reg - 1, reg - 1, binop->inferred_type->byte_size);
	            }

				break;
			}
			case AST_BINARY_SUBSCRIPT: {
				if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
					PUSH_L(tmp, true);
		        	Pipe::handle(&binop->lhs);
					POP_L(tmp);

					Pipe::handle(&binop->rhs);

					auto array_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
					auto array_base_type = static_cast<Ast_Type_Instance*>(array_type->base);
		            auto element_size = array_base_type->byte_size;

		            if (element_size > 1) INST(binop, Mul_Const, reg - 1, element_size);

					this->reg -= 1;
		            INST(binop, Binary, BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64);

		            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    INST(binop, Load, reg - 1, reg - 1, binop->inferred_type->byte_size);
	                }
				} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
					auto struct_type = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
					if (struct_type->is_slice) {
						auto data_decl = struct_type->find_attribute("data");
						if (data_decl) {
							auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
							auto array_base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
				            auto element_size = array_base_type->byte_size;

							PUSH_L(tmp, true);
				        	Pipe::handle(&binop->lhs);
							POP_L(tmp);

							INST(binop, Add_Const, binop->lhs->reg, data_decl->attribute_byte_offset);
							INST(binop, Load, binop->lhs->reg, binop->lhs->reg, ptr_type->byte_size);
							Pipe::handle(&binop->rhs);

				            if (element_size > 1) INST(binop, Mul_Const, binop->lhs->reg, element_size);

				            INST(binop, Binary, BYTECODE_ADD, binop->lhs->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

			                if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			                    INST(binop, Load, binop->lhs->reg, binop->lhs->reg, binop->inferred_type->byte_size);
			                }
						} else abort();
					} else ERROR(binop->lhs, "Struct is not a slice");
				}
				break;
			}
			case AST_BINARY_ASSIGN: {
	            auto size = binop->rhs->inferred_type->byte_size;
				Pipe::handle(&binop->rhs);

				PUSH_L(tmp, true);
	        	Pipe::handle(&binop->lhs);
				POP_L(tmp);

	            this->reg -= 1;
				if (size > INTERP_REGISTER_SIZE) {
	                INST(binop, Copy_Memory, this->reg, this->reg - 1, size);
	            } else {
					INST(binop, Store, this->reg, this->reg - 1, size);
	            }
				break;
			}
			default: {
				Pipe::handle(&binop->lhs);
				Pipe::handle(&binop->rhs);
	            this->reg -= 1;
				auto binop_type = get_bytecode_from_binop(binop->binary_op);
	            auto bytecode_type = bytecode_get_type(binop->lhs->inferred_type);
	            INST(binop, Binary, binop_type, this->reg - 1, this->reg, bytecode_type);

				break;
			}
		}

		if (binop->reg == -1) binop->reg = binop->lhs->reg;
	}

	void handle (Ast_Literal** lit_ptr) {
		auto lit = (*lit_ptr);

		this->push_expression(lit);

		switch (lit->literal_type) {
			case AST_LITERAL_SIGNED_INT:
			case AST_LITERAL_UNSIGNED_INT: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            INST(lit, Set, this->reg, bytecode_type, &lit->int_value);
				break;
			}
			case AST_LITERAL_DECIMAL: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            if (bytecode_type == BYTECODE_TYPE_F32) {
	                auto _tmp = (float) lit->decimal_value;
	                INST(lit, Set, this->reg, bytecode_type, &_tmp);
	            } else {
	                INST(lit, Set, this->reg, bytecode_type, &lit->decimal_value);
	            }
	            break;
	        }
			case AST_LITERAL_STRING: {
				lit->data_offset = g_compiler->interp->constants->add(lit->string_value);
	            INST(lit, Constant_Offset, this->reg, lit->data_offset);
				break;
			}
			default: ERROR(lit, "Literal type to bytecode conversion not supported!");
		}

	    this->reg++;
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		this->push_expression(ident);

		this->push_declaration(ident->declaration, ident->reg);

		if (ident->declaration->is_global()) {
			INST(ident, Global_Offset, reg, ident->declaration->data_offset);
		} else {
			INST(ident, Stack_Offset, reg, ident->declaration->data_offset);
		}

		if (!this->is_left_value && ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			INST(ident, Load, reg, reg, ident->inferred_type->byte_size);
		}

		this->reg++;
	}

    void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

		this->push_expression(func);

		if (func->is_native()) {
			INST(func, Set, this->reg, BYTECODE_TYPE_POINTER, &func->foreign_function_pointer);
		} else {
			INST(func, Set, this->reg, BYTECODE_TYPE_POINTER, &func);
		}

	    this->reg++;
	}

	// TODO: we should try to solve this in a better way...
	// let's see how clang & gcc solve this, maybe we could
	// scan the parameters for different patterns:
	// start from left
	// start from right
	// start at index
	void handle (Ast_Function_Call** call_ptr) {
		auto call = (*call_ptr);

		// If we're not the 1st argument in an expression or argument list,
		// we're overriding the values in previous registers, so we have to
		// store them in the stack and restore them after the call
		auto _tmp = this->reg;
		if (_tmp > 0) {
			INST(call, Stack_Allocate, _tmp * INTERP_REGISTER_SIZE);
			for (uint8_t i = 0; i < _tmp; i++) {
		        INST(call, Stack_Offset, _tmp, this->data_offset);
		        INST(call, Store, _tmp, i, INTERP_REGISTER_SIZE);

				this->data_offset += INTERP_REGISTER_SIZE;
			}
		}

		this->reg = 0;
		PUSH_L(tmp, false);
		for (auto &exp : call->arguments) {
			Pipe::handle(&exp);
		}
		POP_L(tmp);

		INST(call, Call_Setup, DC_CALL_C_X64_WIN64);

		auto bytecode_type = BYTECODE_TYPE_VOID;
		for (uint8_t i = 0; i < call->arguments.size(); i++) {
			auto exp = call->arguments[i];
			if (exp->inferred_type->byte_size > INTERP_REGISTER_SIZE) {
	            bytecode_type = BYTECODE_TYPE_POINTER;
	        } else {
				bytecode_type = bytecode_get_type(exp->inferred_type);
			}
			INST(call, Call_Param, i, bytecode_type);
		}

		PUSH_L(tmp2, false);
	    Pipe::handle(&call->fn);
		POP_L(tmp2);

		auto ret_type = bytecode_get_type(call->inferred_type);
	    INST(call, Call, this->reg - 1, ret_type);
	    if (_tmp != 0) INST(call, Copy, _tmp, 0);
		call->reg = 0;

		// Now we restore the values from previous registers so we can continue
		// with the current expression
		if (_tmp > 0) {
			for (uint8_t i = _tmp; i > 0; i--) {
				this->data_offset -= INTERP_REGISTER_SIZE;

		        INST(call, Stack_Offset, _tmp, this->data_offset);
		        INST(call, Load, i, _tmp, INTERP_REGISTER_SIZE);
			}
		}

		this->reg = _tmp + 1;
	}

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
