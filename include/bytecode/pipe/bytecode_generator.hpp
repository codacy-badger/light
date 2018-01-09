#pragma once

#include "parser/pipes.hpp"
#include "bytecode/interpreter.hpp"

struct Inst_Jump;

#define ERROR(node, ...) report_error_stop(&node->location, __VA_ARGS__)

#define PUSH_L(var_name, value) auto var_name = this->is_left_value; this->is_left_value = value
#define POP_L(var_name) this->is_left_value = var_name

struct Bytecode_Generator : Pipe {
	size_t stack_offset = 0;

	vector<Instruction*>* bytecode = NULL;
	bool is_left_value = false;
	uint8_t reg = 0;

	vector<Inst_Jump*> pending_breaks;

	PIPE_NAME(Bytecode_Generator)

	void add_instruction (Ast* node, Instruction* intruction) {
	    intruction->filename = node->location.filename;
	    intruction->line = node->location.line;
	    this->bytecode->push_back(intruction);
	}

	void handle (Ast_Note**) { /* Notes don't produce any bytecode by themselves */ }

    void handle (Ast_Statement** stm_ptr) {
		this->reg = 0;
		this->is_left_value = false;
		Pipe::handle(stm_ptr);
	}

	void handle (Ast_Block** block_ptr) {
		for (auto &stm : (*block_ptr)->list) {
			this->handle(&stm);
		}
	}

	void handle (Ast_Return** ret_ptr) {
		PUSH_L(tmp, false);
	    Pipe::handle(ret_ptr);
		POP_L(tmp);
	    this->add_instruction((*ret_ptr), new Inst_Return());
	}

	void handle (Ast_If** _if_ptr) {
		auto _if = (*_if_ptr);

		PUSH_L(tmp, false);
		Pipe::handle(&_if->condition);
		POP_L(tmp);

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

		PUSH_L(tmp, false);
		Pipe::handle(&_while->condition);
		POP_L(tmp);

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
	            decl->stack_offset = this->stack_offset;
	            this->stack_offset += decl_type->byte_size;

	            this->add_instruction(decl, new Inst_Stack_Allocate(decl_type->byte_size));
	            this->add_instruction(decl, new Inst_Stack_Offset(free_reg, decl->stack_offset));
	            if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
	                this->add_instruction(decl, new Inst_Copy_Memory(free_reg, i, decl_type->byte_size));
	            } else {
	                this->add_instruction(decl, new Inst_Store(free_reg, i, decl_type->byte_size));
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
	    			auto _tmp = this->stack_offset;
	    			this->fill(func);
	                this->stack_offset = _tmp;
	            }
			}
	    } else {
			auto ty_decl = static_cast<Ast_Type_Instance*>(decl->type);
			if (decl->decl_flags & AST_DECL_FLAG_GLOBAL) {
				decl->global_offset = g_compiler->interp->globals->add(ty_decl->byte_size);
			} else {
	            bool zero_init = !(decl->decl_flags & AST_DECL_FLAG_UNINIT);
	            zero_init = zero_init && !decl->expression;
	            this->add_instruction(decl, new Inst_Stack_Allocate(ty_decl->byte_size, zero_init));

				decl->stack_offset = this->stack_offset;
				this->stack_offset += ty_decl->byte_size;
				if (decl->expression) {
					PUSH_L(tmp, false);
					Pipe::handle(&decl->expression);
					POP_L(tmp);

	                this->add_instruction(decl, new Inst_Stack_Offset(1, decl->stack_offset));
					if (ty_decl->byte_size > INTERP_REGISTER_SIZE) {
		                this->add_instruction(decl, new Inst_Copy_Memory(1, 0, ty_decl->byte_size));
		            } else {
						this->add_instruction(decl, new Inst_Store(1, 0, ty_decl->byte_size));
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

		auto type_from = bytecode_get_type(cast->value->inferred_type);
		auto type_to = bytecode_get_type(cast->inferred_type);
		this->add_instruction(cast, new Inst_Cast(this->reg - 1, type_from, type_to));
	}

	void handle (Ast_Literal** lit_ptr) {
		auto lit = (*lit_ptr);

		switch (lit->literal_type) {
			case AST_LITERAL_SIGNED_INT:
			case AST_LITERAL_UNSIGNED_INT: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            this->add_instruction(lit, new Inst_Set(this->reg++, bytecode_type, &lit->int_value));
				break;
			}
			case AST_LITERAL_DECIMAL: {
				auto bytecode_type = bytecode_get_type(lit->inferred_type);
	            if (bytecode_type == BYTECODE_TYPE_F32) {
	                auto _tmp = (float) lit->decimal_value;
	                this->add_instruction(lit, new Inst_Set(this->reg++, bytecode_type, &_tmp));
	            } else {
	                this->add_instruction(lit, new Inst_Set(this->reg++, bytecode_type, &lit->decimal_value));
	            }
	            break;
	        }
			case AST_LITERAL_STRING: {
				lit->data_offset = g_compiler->interp->constants->add(lit->string_value);
	            this->add_instruction(lit, new Inst_Constant_Offset(this->reg++, lit->data_offset));
				break;
			}
			default: ERROR(lit, "Literal type to bytecode conversion not supported!");
		}
	}

	uint8_t get_bytecode_from_unop (Ast_Unary_Type unop) {
		switch (unop) {
			case AST_UNARY_NEGATE:        return BYTECODE_ARITHMETIC_NEGATE;
			case AST_UNARY_NOT:           return BYTECODE_LOGICAL_NEGATE;
		}
		return BYTECODE_NOOP;
	}

	void handle (Ast_Unary** unop_ptr) {
		auto unop = (*unop_ptr);

		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
				PUSH_L(tmp, false);
	        	Pipe::handle(&unop->exp);
				POP_L(tmp);

				auto unop_type = get_bytecode_from_unop(unop->unary_op);
	            auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
	            this->add_instruction(unop, new Inst_Unary(unop_type, this->reg - 1, bytecode_type));
				break;
			}
			case AST_UNARY_NEGATE: {
				PUSH_L(tmp, false);
	        	Pipe::handle(&unop->exp);
				POP_L(tmp);

				auto unop_type = get_bytecode_from_unop(unop->unary_op);
	            auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
				auto result_type = bytecode_unsigned_to_signed(bytecode_type);
				if (bytecode_type != result_type) {
					this->add_instruction(unop, new Inst_Cast(this->reg - 1, bytecode_type, result_type));
				}
	            this->add_instruction(unop, new Inst_Unary(unop_type, this->reg - 1, result_type));
				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				PUSH_L(tmp, false);
	            Pipe::handle(&unop->exp);
				POP_L(tmp);

				if (!this->is_left_value) {
					this->add_instruction(unop, new Inst_Load(reg - 1, reg - 1, unop->inferred_type->byte_size));
				}
	            break;
	        }
	        case AST_UNARY_REFERENCE: {
				PUSH_L(tmp, true);
	            Pipe::handle(&unop->exp);
				POP_L(tmp);

	            break;
	        }
			default: return;
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
	                this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, type_def->byte_size));
	            }

	            auto ident = static_cast<Ast_Ident*>(binop->rhs);
	            auto decl = ident->declaration;
				if (decl->attribute_byte_offset != 0) {
					this->add_instruction(binop, new Inst_Add_Const(reg - 1, decl->attribute_byte_offset));
				}

	            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
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

		            if (element_size > 1) {
						this->add_instruction(binop, new Inst_Mul_Const(reg - 1, element_size));
		            }

					this->reg -= 1;
		            this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));

		            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
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

							this->add_instruction(binop, new Inst_Add_Const(reg - 1, data_decl->attribute_byte_offset));
							this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, ptr_type->byte_size));
							Pipe::handle(&binop->rhs);

				            if (element_size > 1) {
								this->add_instruction(binop, new Inst_Mul_Const(reg - 1, element_size));
				            }

							this->reg -= 1;
				            this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));

			                if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			                    this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
			                }
						} else ERROR(binop, "Slice type doesn't have data attribute");
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
	                this->add_instruction(binop, new Inst_Copy_Memory(reg, reg - 1, size));
	            } else {
					this->add_instruction(binop, new Inst_Store(reg, reg - 1, size));
	            }
				break;
			}
			default: {
				Pipe::handle(&binop->lhs);
				Pipe::handle(&binop->rhs);
	            this->reg -= 1;
				auto binop_type = get_bytecode_from_binop(binop->binary_op);
	            auto bytecode_type = bytecode_get_type(binop->lhs->inferred_type);
	            this->add_instruction(binop, new Inst_Binary(binop_type, reg - 1, reg, bytecode_type));

				break;
			}
		}
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (ident->declaration->decl_flags && AST_DECL_FLAG_GLOBAL) {
			this->add_instruction(ident, new Inst_Global_Offset(reg, ident->declaration->global_offset));
		} else {
			this->add_instruction(ident, new Inst_Stack_Offset(reg, ident->declaration->stack_offset));
		}
		if (!this->is_left_value && ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			this->add_instruction(ident, new Inst_Load(reg, reg, ident->inferred_type->byte_size));
		}
		this->reg++;
	}

    void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

		if (func->is_native()) {
			auto module = os_get_module(func->foreign_module_name);
			if (module) {
				auto function_pointer = os_get_function(module, func->foreign_function_name);
				if (!function_pointer) {
	                ERROR(func, "Function '%s' not found in module '%s'!",
						func->foreign_function_name, func->foreign_module_name);
				} else {
					this->add_instruction(func, new Inst_Set(this->reg, BYTECODE_TYPE_POINTER, &function_pointer));
				}
			} else ERROR(func, "Module '%s' not found!", func->foreign_module_name);
		} else {
			this->add_instruction(func, new Inst_Set(this->reg, BYTECODE_TYPE_POINTER, &func));
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
			this->add_instruction(call, new Inst_Stack_Allocate(_tmp * INTERP_REGISTER_SIZE));
			for (uint8_t i = 0; i < _tmp; i++) {
		        this->add_instruction(call, new Inst_Stack_Offset(_tmp, this->stack_offset));
		        this->add_instruction(call, new Inst_Store(_tmp, i, INTERP_REGISTER_SIZE));

				this->stack_offset += INTERP_REGISTER_SIZE;
			}
		}

		this->reg = 0;
		for (auto &exp : call->arguments) {
			PUSH_L(tmp, false);
			Pipe::handle(&exp);
			POP_L(tmp);
		}

		this->add_instruction(call, new Inst_Call_Setup(DC_CALL_C_X64_WIN64));

		auto bytecode_type = BYTECODE_TYPE_VOID;
		for (uint8_t i = 0; i < call->arguments.size(); i++) {
			auto exp = call->arguments[i];
			if (exp->inferred_type->byte_size > INTERP_REGISTER_SIZE) {
	            bytecode_type = BYTECODE_TYPE_POINTER;
	        } else {
				bytecode_type = bytecode_get_type(exp->inferred_type);
			}
			this->add_instruction(call, new Inst_Call_Param(i, bytecode_type));
		}

		PUSH_L(tmp, false);
	    Pipe::handle(&call->fn);
		POP_L(tmp);

		auto ret_type = bytecode_get_type(call->inferred_type);
	    this->add_instruction(call, new Inst_Call(this->reg - 1, ret_type));
	    if (_tmp != 0) {
	        this->add_instruction(call, new Inst_Copy(_tmp, 0));
	    }

		// Now we restore the values from previous registers so we can continue
		// with the current expression
		if (_tmp > 0) {
			for (uint8_t i = _tmp; i > 0; i--) {
				this->stack_offset -= INTERP_REGISTER_SIZE;

		        this->add_instruction(call, new Inst_Stack_Offset(_tmp, this->stack_offset));
		        this->add_instruction(call, new Inst_Load(i, _tmp, INTERP_REGISTER_SIZE));
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
};
