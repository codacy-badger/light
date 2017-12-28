#include "bytecode/pipe/bytecode_generator.hpp"

#include "compiler.hpp"

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm);
    this->to_next(stm);
}

void Bytecode_Generator::add_instruction (Ast* node, Instruction* intruction) {
    intruction->filename = node->filename;
    intruction->line = node->line;
    this->bytecode->push_back(intruction);
}

void Bytecode_Generator::gen (Ast_Statement* stm) {
	this->current_register = 0;
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: this->gen(static_cast<Ast_Declaration*>(stm)); break;
		case AST_STATEMENT_RETURN: this->gen(static_cast<Ast_Return*>(stm)); break;
		case AST_STATEMENT_IF: this->gen(static_cast<Ast_If*>(stm)); break;
		case AST_STATEMENT_WHILE: this->gen(static_cast<Ast_While*>(stm)); break;
		case AST_STATEMENT_BREAK: this->gen(static_cast<Ast_Break*>(stm)); break;
		case AST_STATEMENT_EXPRESSION: this->gen(static_cast<Ast_Expression*>(stm)); break;
		case AST_STATEMENT_BLOCK: {
            auto block = static_cast<Ast_Block*>(stm);
            for (auto _stm : block->list) this->gen(_stm);
            break;
        }
        default: break;
    }
}

void Bytecode_Generator::gen (Ast_Return* ret) {
	if (ret->exp) {
	    this->gen(ret->exp);
	    if (this->current_register != 1) {
	        this->add_instruction(ret, new Inst_Copy(0, this->current_register - 1));
	    }
	}
    this->add_instruction(ret, new Inst_Return());
}

void Bytecode_Generator::gen (Ast_If* _if) {
	this->gen(_if->condition);

	auto inst = new Inst_Jump_If_False(0);
	this->add_instruction(_if, inst);

	auto index1 = this->bytecode->size();
	this->gen(_if->then_statement);
	inst->offset = this->bytecode->size() - index1;
	if (_if->else_statement) {
		inst->offset += 1;
		auto inst2 = new Inst_Jump();
		this->add_instruction(_if, inst2);

		index1 = this->bytecode->size();
		this->gen(_if->else_statement);
		inst2->offset = this->bytecode->size() - index1;
	}
}

void Bytecode_Generator::gen (Ast_While* _while) {
	auto index1 = this->bytecode->size();

	this->gen(_while->condition);

	auto jmp1 = new Inst_Jump_If_False(0);
	this->add_instruction(_while, jmp1);
	auto index2 = this->bytecode->size();

	this->gen(_while->statement);

	auto jmp2 = new Inst_Jump();
	this->add_instruction(_while, jmp2);
	jmp2->offset = index1 - this->bytecode->size();
	jmp1->offset = this->bytecode->size() - index2;

	this->update_pending_breaks();
}

void Bytecode_Generator::gen (Ast_Break* _break) {
	auto jump = new Inst_Jump();
	this->add_instruction(_break, jump);
	this->pending_breaks.push_back(jump);
	jump->offset = this->bytecode->size();
}

void Bytecode_Generator::fill (Ast_Function* fn) {
	if (fn->scope && fn->bytecode.size() == 0) {
        auto _tmp = this->bytecode;
        this->bytecode = &fn->bytecode;

        auto free_reg = (uint8_t) fn->type->parameter_decls.size();
        for (uint8_t i = 0; i < fn->type->parameter_decls.size(); i++) {
            auto decl = fn->type->parameter_decls[i];

            auto decl_type = static_cast<Ast_Type_Definition*>(decl->type);
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

		this->current_register = 0;
		this->gen(fn->scope);

		if (fn->type->return_type == Light_Compiler::inst->type_def_void) {
			if (this->bytecode->size() == 0) {
				this->add_instruction(fn->scope, new Inst_Return());
			} else {
				auto last_inst = this->bytecode->back();
				if (last_inst->bytecode != BYTECODE_RETURN) {
					this->add_instruction(fn->scope, new Inst_Return());
				}
			}
		}

        this->bytecode = _tmp;
	}
}

void Bytecode_Generator::gen (Ast_Declaration* decl) {
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
		auto ty_decl = static_cast<Ast_Type_Definition*>(decl->type);
		if (decl->decl_flags & AST_DECL_FLAG_GLOBAL) {
			decl->global_offset = Light_Compiler::inst->interp->globals->add(ty_decl->byte_size);
		} else {
            bool zero_init = !(decl->decl_flags & AST_DECL_FLAG_UNINIT);
            zero_init = zero_init && !decl->expression;
            this->add_instruction(decl, new Inst_Stack_Allocate(ty_decl->byte_size, zero_init));

			decl->stack_offset = this->stack_offset;
			this->stack_offset += ty_decl->byte_size;
			if (decl->expression) {
				this->gen(decl->expression);
                this->add_instruction(decl, new Inst_Stack_Offset(1, decl->stack_offset));
                this->add_instruction(decl, new Inst_Store(1, 0, ty_decl->byte_size));
			}
		}
	}
}

void Bytecode_Generator::gen (Ast_Expression* exp, bool left_value) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_CAST: return this->gen(static_cast<Ast_Cast*>(exp));
        case AST_EXPRESSION_LITERAL: return this->gen(static_cast<Ast_Literal*>(exp));
		case AST_EXPRESSION_UNARY: return this->gen(static_cast<Ast_Unary*>(exp), left_value);
        case AST_EXPRESSION_BINARY: return this->gen(static_cast<Ast_Binary*>(exp), left_value);
        case AST_EXPRESSION_IDENT: return this->gen(static_cast<Ast_Ident*>(exp), left_value);
        case AST_EXPRESSION_CALL: return this->gen(static_cast<Ast_Function_Call*>(exp));
        case AST_EXPRESSION_FUNCTION: return this->gen(static_cast<Ast_Function*>(exp));
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Cast* cast) {
	this->gen(cast->value);
	auto type_from = bytecode_get_type(cast->value->inferred_type);
	auto type_to = bytecode_get_type(cast->inferred_type);
	this->add_instruction(cast, new Inst_Cast(this->current_register - 1, type_from, type_to));
}

void Bytecode_Generator::gen (Ast_Literal* lit) {
	switch (lit->literal_type) {
		case AST_LITERAL_SIGNED_INT:
		case AST_LITERAL_UNSIGNED_INT: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
            this->add_instruction(lit, new Inst_Set(this->current_register++, bytecode_type, &lit->int_value));
			break;
		}
		case AST_LITERAL_DECIMAL: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
            if (bytecode_type == BYTECODE_TYPE_F32) {
                auto _tmp = (float) lit->decimal_value;
                this->add_instruction(lit, new Inst_Set(this->current_register++, bytecode_type, &_tmp));
            } else {
                this->add_instruction(lit, new Inst_Set(this->current_register++, bytecode_type, &lit->decimal_value));
            }
            break;
        }
		case AST_LITERAL_STRING: {
			lit->data_offset = Light_Compiler::inst->interp->constants->add(lit->string_value);
            this->add_instruction(lit, new Inst_Constant_Offset(this->current_register++, lit->data_offset));
			break;
		}
		default: {
			Light_Compiler::inst->error_stop(lit, "Literal type to bytecode conversion not supported!");
		}
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

uint8_t count_dereferences (Ast_Type_Definition* type_def) {
    uint8_t deref_count = 0;
    while (type_def->typedef_type == AST_TYPEDEF_POINTER) {
        auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
        type_def = static_cast<Ast_Type_Definition*>(ptr_type->base);
        deref_count += 1;
    }
    return deref_count;
}

void Bytecode_Generator::gen (Ast_Binary* binop, bool left_value) {
	switch (binop->binary_op) {
		case AST_BINARY_ATTRIBUTE: {
        	this->gen(binop->lhs, true);

            auto ident = static_cast<Ast_Ident*>(binop->rhs);
			auto reg = this->current_register;
            auto decl = ident->declaration;

            // We follow the pointer until we hit a non-pointer type
            auto type_def = binop->lhs->inferred_type;
            while (type_def->typedef_type == AST_TYPEDEF_POINTER) {
                auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
                type_def = static_cast<Ast_Type_Definition*>(ptr_type->base);
                this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, type_def->byte_size));
            }

			if (decl->attribute_byte_offset != 0) {
	            this->add_instruction(binop, new Inst_Set(reg, BYTECODE_TYPE_U16, &decl->attribute_byte_offset));
	            this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));
			}

            if (!left_value) {
                if (binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
                    this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
                } else {
                    // TODO: this value should be hidden into a reg by pointer
                    Light_Compiler::inst->error_stop(binop, "Value of identifier is bigger than a register!");
                }
            }

			break;
		}
		case AST_BINARY_SUBSCRIPT: {
			if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
				auto array_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
				auto array_base_type = static_cast<Ast_Type_Definition*>(array_type->base);
	            auto element_size = array_base_type->byte_size;

	        	this->gen(binop->lhs, true);
				this->gen(binop->rhs, left_value);

				auto reg = this->current_register;
	            if (element_size > 1) {
	                this->add_instruction(binop, new Inst_Set(this->current_register, BYTECODE_TYPE_U64, &element_size));
	                this->add_instruction(binop, new Inst_Binary(BYTECODE_MUL, reg - 1, reg, BYTECODE_TYPE_U64));
	            }

				reg = --this->current_register;
	            this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));

	            if (!left_value) {
	                if (binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
	                } else {
	                    // TODO: this value should be hidden into a reg by pointer
	                    Light_Compiler::inst->error_stop(binop, "Value of identifier is bigger than a register!");
	                }
	            }
			} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
				auto struct_type = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
				if (struct_type->is_slice) {
					auto data_decl = struct_type->find_attribute("data");
					if (data_decl) {
						auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
						auto array_base_type = static_cast<Ast_Type_Definition*>(ptr_type->base);
			            auto element_size = array_base_type->byte_size;

			        	this->gen(binop->lhs, true);
						auto reg = this->current_register;
		                this->add_instruction(binop, new Inst_Set(reg, BYTECODE_TYPE_U64, &data_decl->attribute_byte_offset));
		                this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));
						this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, ptr_type->byte_size));
						this->gen(binop->rhs, left_value);

						reg = this->current_register;
			            if (element_size > 1) {
			                this->add_instruction(binop, new Inst_Set(reg, BYTECODE_TYPE_U64, &element_size));
			                this->add_instruction(binop, new Inst_Binary(BYTECODE_MUL, reg - 1, reg, BYTECODE_TYPE_U64));
			            }

						reg -= 1;
			            this->add_instruction(binop, new Inst_Binary(BYTECODE_ADD, reg - 1, reg, BYTECODE_TYPE_U64));

			            if (!left_value) {
			                if (binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			                    this->add_instruction(binop, new Inst_Load(reg - 1, reg - 1, binop->inferred_type->byte_size));
			                } else {
			                    // TODO: this value should be hidden into a reg by pointer
			                    Light_Compiler::inst->error_stop(binop, "Value of identifier is bigger than a register!");
			                }
			            }
					} else {
						Light_Compiler::inst->error_stop(binop, "Slice type doesn't have data attribute");
					}
				} else {
					Light_Compiler::inst->error_stop(binop->lhs, "Struct is not slice");
				}
			}
			break;
		}
		case AST_BINARY_ASSIGN: {
            auto size = binop->rhs->inferred_type->byte_size;
			this->gen(binop->rhs, left_value);
        	this->gen(binop->lhs, true);
            this->current_register--;
			auto reg = this->current_register;
            this->add_instruction(binop, new Inst_Store(reg, reg - 1, size));
			break;
		}
		default: {
			this->gen(binop->lhs, left_value);
			this->gen(binop->rhs, left_value);
            this->current_register--;
			auto reg = this->current_register;
			auto binop_type = get_bytecode_from_binop(binop->binary_op);
            auto bytecode_type = bytecode_get_type(binop->lhs->inferred_type);
            this->add_instruction(binop, new Inst_Binary(binop_type, reg - 1, reg, bytecode_type));

			break;
		}
	}
}

uint8_t get_bytecode_from_unop (Ast_Unary_Type unop) {
	switch (unop) {
		case AST_UNARY_NEGATE:        return BYTECODE_ARITHMETIC_NEGATE;
		case AST_UNARY_NOT:           return BYTECODE_LOGICAL_NEGATE;
	}
	return BYTECODE_NOOP;
}

void Bytecode_Generator::gen (Ast_Unary* unop, bool left_value) {
	switch (unop->unary_op) {
		case AST_UNARY_NOT: {
        	this->gen(unop->exp);
			auto unop_type = get_bytecode_from_unop(unop->unary_op);
            auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
            this->add_instruction(unop, new Inst_Unary(unop_type, this->current_register - 1, bytecode_type));
			break;
		}
		case AST_UNARY_NEGATE: {
        	this->gen(unop->exp);
			auto unop_type = get_bytecode_from_unop(unop->unary_op);
            auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
			auto result_type = bytecode_unsigned_to_signed(bytecode_type);
			if (bytecode_type != result_type) {
				this->add_instruction(unop, new Inst_Cast(this->current_register - 1, bytecode_type, result_type));
			}
            this->add_instruction(unop, new Inst_Unary(unop_type, this->current_register - 1, result_type));
			break;
		}
        case AST_UNARY_DEREFERENCE: {
            this->gen(unop->exp, left_value);
            uint8_t reg = this->current_register - 1;
            this->add_instruction(unop, new Inst_Load(reg, reg, unop->exp->inferred_type->byte_size));
            break;
        }
        case AST_UNARY_REFERENCE: {
            this->gen(unop->exp, true);
            break;
        }
		default: return;
	}
}

void Bytecode_Generator::gen (Ast_Ident* ident, bool left_value) {
	auto reg = this->current_register++;
	if (ident->declaration->decl_flags && AST_DECL_FLAG_GLOBAL) {
		this->add_instruction(ident, new Inst_Global_Offset(reg, ident->declaration->global_offset));
	} else {
		this->add_instruction(ident, new Inst_Stack_Offset(reg, ident->declaration->stack_offset));
	}
	if (!left_value) {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			this->add_instruction(ident, new Inst_Load(reg, reg, ident->inferred_type->byte_size));
		} else {
			//Light_Compiler::inst->error_stop(ident, "Value of identifier is bigger than a register!");
		}
	}
}

void Bytecode_Generator::gen (Ast_Function* func) {
    if (func->foreign_module_name) {
		auto ffunctions = Light_Compiler::inst->interp->foreign_functions;
		auto module = ffunctions->get_or_add_module(func->foreign_module_name);
		if (module) {
			auto function_pointer = ffunctions->get_or_add_function(module, func->foreign_function_name);
			if (function_pointer) {
	            this->add_instruction(func, new Inst_Set(this->current_register, BYTECODE_TYPE_POINTER, &function_pointer));
				this->current_register++;
			}
		} else {
			Light_Compiler::inst->error_stop(func, "Module '%s' not found!", func->foreign_module_name);
		}
	} else {
		Light_Compiler::inst->error_stop(func, "Internal functions cannot be referenced...");
	}
}

// TODO: we should try to solve this in a better way...
// let's see how clang & gcc solve this, maybe we could
// scan the parameters for different patterns:
// start from left
// start from right
// start at index
void Bytecode_Generator::gen (Ast_Function_Call* call) {
	// If we're not the 1st argument in an expression or argument list,
	// we're overriding the values in previous registers, so we have to
	// store them in the stack and restore them after the call
	auto _tmp = this->current_register;
	if (_tmp > 0) {
		this->add_instruction(call, new Inst_Stack_Allocate(_tmp * INTERP_REGISTER_SIZE));
		for (uint8_t i = 0; i < _tmp; i++) {
	        this->add_instruction(call, new Inst_Stack_Offset(_tmp, this->stack_offset));
	        this->add_instruction(call, new Inst_Store(_tmp, i, INTERP_REGISTER_SIZE));

			this->stack_offset += INTERP_REGISTER_SIZE;
		}
	}

	this->current_register = 0;
	if (call->args) {
		for (auto exp : call->args->values) {
            if (exp->inferred_type->byte_size > INTERP_REGISTER_SIZE) {
                this->gen(exp, true);
            } else this->gen(exp);
		}
	}

	this->add_instruction(call, new Inst_Call_Setup(DC_CALL_C_X64_WIN64));

	if (call->args) {
		auto bytecode_type = BYTECODE_TYPE_VOID;
		for (uint8_t i = 0; i < call->args->values.size(); i++) {
			auto exp = call->args->values[i];
			if (exp->inferred_type->byte_size > INTERP_REGISTER_SIZE) {
                bytecode_type = BYTECODE_TYPE_POINTER;
            } else {
				bytecode_type = bytecode_get_type(exp->inferred_type);
			}
			this->add_instruction(call, new Inst_Call_Param(i, bytecode_type));
		}
	}

	auto func = static_cast<Ast_Function*>(call->fn);
    this->add_instruction(call, new Inst_Call(reinterpret_cast<size_t>(func)));
	if (_tmp != 0) {
	    this->add_instruction(call, new Inst_Copy(_tmp, 0));
	}

	// Now we restore the values from previous registers so we can continue
	// with the current expression
	if (_tmp > 0) {
		auto max_size = this->stack_offset;
		for (uint8_t i = _tmp; i > 0; i--) {
			this->stack_offset -= INTERP_REGISTER_SIZE;

	        this->add_instruction(call, new Inst_Stack_Offset(_tmp, this->stack_offset));
	        this->add_instruction(call, new Inst_Load(i, _tmp, INTERP_REGISTER_SIZE));
		}
        this->add_instruction(call, new Inst_Stack_Free(max_size - this->stack_offset));
	}

	this->current_register = _tmp + 1;
}

void Bytecode_Generator::update_pending_breaks () {
	while (!this->pending_breaks.empty()) {
		auto jump = this->pending_breaks.back();
		jump->offset = this->bytecode->size() - jump->offset;
		this->pending_breaks.pop_back();
	}
}
