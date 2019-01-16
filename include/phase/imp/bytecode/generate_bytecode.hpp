#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"
#include "ast/ast_factory.hpp"

#include "bytecode/instructions.hpp"
#include "bytecode/interpreter.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"

#include "util/logger.hpp"

#define INST(node, name, ...) this->add_instruction(node, new Inst_##name(__VA_ARGS__));

struct Generate_Bytecode : Phase, Ast_Navigator {
    Bytecode_Constants* constants = new Bytecode_Constants();
    Bytecode_Globals* globals = new Bytecode_Globals();

	Ast_Declaration* reg_declarations[INTERP_REGISTER_SIZE] = {};
	bool reserved[INTERP_REGISTER_SIZE] = {};
	size_t data_offset = 0;

	std::vector<Instruction*>* bytecode = NULL;
	bool is_left_value = false;

	std::vector<Inst_Jump*> pending_breaks;

	// for metrics
	size_t instruction_count = 0;

	Generate_Bytecode () : Phase("Generate Bytecode", CE_BYTECODE_GENERATE, true) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

	void add_instruction (Ast* node, Instruction* intruction) {
	    intruction->location = node->location;
	    this->bytecode->push_back(intruction);
		this->instruction_count += 1;
	}

	void handle (const char*) { /* Notes don't produce any bytecode by themselves */ }

    void handle (Ast_Statement* stm) {
		this->is_left_value = false;
		Ast_Navigator::ast_handle(stm);
		//this->clear_registers();
	}

	void handle (Ast_Scope* scope) {
		for (auto stm : scope->statements) {
			this->handle(stm);
		}
	}

	void handle (Ast_Return* ret) {
	    Ast_Navigator::ast_handle(ret);

		if (ret->expression) {
			auto bytecode_type = bytecode_get_type(ret->expression->inferred_type);
			INST(ret, Return, ret->expression->reg, bytecode_type);
		} else INST(ret, Return);
	}

	void handle (Ast_If* _if) {
		Ast_Navigator::ast_handle(_if->condition);

		auto inst = new Inst_Jump_If_False(_if->condition->reg);
		this->add_instruction(_if, inst);

		auto index1 = this->bytecode->size();
		Ast_Navigator::ast_handle(_if->then_scope);
		inst->offset = this->bytecode->size() - index1;
		if (_if->else_scope) {
			inst->offset += 1;
			auto inst2 = new Inst_Jump();
			this->add_instruction(_if, inst2);

			index1 = this->bytecode->size();
			Ast_Navigator::ast_handle(_if->else_scope);
			inst2->offset = this->bytecode->size() - index1;
		}
	}

	void handle (Ast_While* _while) {
		auto index1 = this->bytecode->size();

		Ast_Navigator::ast_handle(_while->condition);

		auto jmp1 = new Inst_Jump_If_False(_while->condition->reg);
		this->add_instruction(_while, jmp1);
		auto index2 = this->bytecode->size();

		Ast_Navigator::ast_handle(_while->scope);

		auto jmp2 = new Inst_Jump();
		this->add_instruction(_while, jmp2);
		jmp2->offset = index1 - this->bytecode->size();
		jmp1->offset = this->bytecode->size() - index2;

		this->update_pending_breaks();
	}

	void handle (Ast_Break* _break) {
		auto jump = new Inst_Jump();
		this->add_instruction(_break, jump);
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
					decl->bytecode_stack_offset = this->data_offset;
					this->data_offset += decl_type->byte_size;

					INST(decl, Stack_Offset, free_reg, decl->bytecode_stack_offset);
					if (decl_type->byte_size > INTERP_REGISTER_SIZE) {
						INST(decl, Copy_Memory, free_reg, i, decl_type->byte_size);
					} else {
						INST(decl, Store, free_reg, i, decl_type->byte_size);
					}
				}
	        }

			this->handle(func->scope);

			if (func->type->ret_type == Types::type_void) {
				if (this->bytecode->size() == 0) {
					this->add_instruction(func->scope, new Inst_Return());
				} else {
					auto last_inst = this->bytecode->back();
					if (last_inst->code != BYTECODE_RETURN) {
						this->add_instruction(func->scope, new Inst_Return());
					}
				}
			}

	        this->bytecode = _tmp;
		}
	}

    void handle (Ast_Declaration* decl) {
	    if (decl->is_constant) {
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
			if (this->current_scope->is_global()) {
				decl->bytecode_global_offset = this->globals->allocate(decl_type->byte_size);
			} else {
				if (decl->expression) Ast_Navigator::ast_handle(decl->expression);

				if (decl->is_spilled) {
					INST(decl, Stack_Allocate, decl_type->byte_size);
					decl->bytecode_stack_offset = this->data_offset;
					this->data_offset += decl_type->byte_size;

					if (decl->expression) {
						uint8_t free_reg = decl->expression->reg + 1;
						INST(decl, Stack_Offset, free_reg, decl->bytecode_stack_offset);
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

	void handle (Ast_Expression* exp) {
		if (this->is_left_value != false) {
			auto tmp = this->is_left_value;
			this->is_left_value = false;
			Ast_Navigator::ast_handle(exp);
			this->is_left_value = tmp;
		} else Ast_Navigator::ast_handle(exp);
	}

	void handle_left (Ast_Expression* exp) {
		if (this->is_left_value != true) {
			auto tmp = this->is_left_value;
			this->is_left_value = true;
			Ast_Navigator::ast_handle(exp);
			this->is_left_value = tmp;
		} else Ast_Navigator::ast_handle(exp);
	}

	void handle (Ast_Run* run) {
		if (run->bytecode.size() == 0) {
	        auto _tmp = this->bytecode;
	        this->bytecode = &run->bytecode;

			this->handle(run->expression);

	        this->bytecode = _tmp;
		}
	}

	void handle (Ast_Cast* cast) {
		Ast_Navigator::ast_handle(cast->value);

		auto type_from = bytecode_get_type(cast->value->inferred_type);
		auto type_to = bytecode_get_type(cast->inferred_type);
		if (type_from != type_to) {
			INST(cast, Cast, cast->reg, cast->value->reg, type_from, type_to);
		} else if (cast->reg != cast->value->reg) {
			INST(cast, Copy, cast->reg, cast->value->reg);
		}
	}

	void handle (Ast_Unary* unop) {
		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
				Ast_Navigator::ast_handle(unop->exp);
				auto unop_type = get_bytecode_from_unop(unop->unary_op);
				auto bytecode_type = bytecode_get_type(unop->exp->inferred_type);
	            INST(unop, Unary, unop_type, unop->reg, unop->exp->reg, bytecode_type);
				break;
			}
			case AST_UNARY_NEGATE: {
				Ast_Navigator::ast_handle(unop->exp);
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
				this->handle_left(unop->exp);
				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				Ast_Navigator::ast_handle(unop->exp);
				if (!this->is_left_value) {
					INST(unop, Load, unop->reg, unop->exp->reg, unop->inferred_type->byte_size);
				}
	            break;
	        }
			default: abort();
		}
	}

	void handle (Ast_Binary* binop) {
		switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
	        	this->handle_left(binop->lhs);

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
		        	this->handle_left(binop->lhs);

					Ast_Navigator::ast_handle(binop->rhs);

					auto array_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
					auto array_base_type = static_cast<Ast_Type_Instance*>(array_type->base);
		            auto element_size = array_base_type->byte_size;

		            if (element_size > 1) INST(binop, Mul_Const, binop->rhs->reg, element_size);

		            INST(binop, Binary, BYTECODE_ADD, binop->reg, binop->lhs->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

		            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    INST(binop, Load, binop->reg, binop->reg, binop->inferred_type->byte_size);
	                }
				} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
					// TODO: remove this once we have operator overloading ([])
					auto struct_type = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
					if (struct_type->is_slice) {
						auto data_decl = struct_type->find_attribute("data");
						if (data_decl) {
							auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
							auto array_base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
				            auto element_size = array_base_type->byte_size;

				        	this->handle_left(binop->lhs);
							INST(binop, Add_Const, binop->reg, binop->lhs->reg, data_decl->attribute_byte_offset);
							INST(binop, Load, binop->reg, binop->reg, ptr_type->byte_size);

							Ast_Navigator::ast_handle(binop->rhs);

				            if (element_size > 1) INST(binop, Mul_Const, binop->reg, element_size);

				            INST(binop, Binary, BYTECODE_ADD, binop->reg, binop->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

			                if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			                    INST(binop, Load, binop->reg, binop->reg, binop->inferred_type->byte_size);
			                }
						} else abort();
					} else Logger::error(binop->lhs, "Struct is not a slice");
				} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_POINTER) {
		        	this->handle_left(binop->lhs);

					Ast_Navigator::ast_handle(binop->rhs);

					auto ptr_type = static_cast<Ast_Pointer_Type*>(binop->lhs->inferred_type);
					auto ptr_base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
		            auto element_size = ptr_base_type->byte_size;

		            if (element_size > 1) INST(binop, Mul_Const, binop->rhs->reg, element_size);

		            INST(binop, Binary, BYTECODE_ADD, binop->reg, binop->lhs->reg, binop->rhs->reg, BYTECODE_TYPE_U64);

		            if (!this->is_left_value && binop->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
	                    INST(binop, Load, binop->reg, binop->reg, binop->inferred_type->byte_size);
	                }
				}
				break;
			}
			case AST_BINARY_ASSIGN: {
	            auto size = binop->rhs->inferred_type->byte_size;
				Ast_Navigator::ast_handle(binop->rhs);

	        	this->handle_left(binop->lhs);

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
				Ast_Navigator::ast_handle(binop->lhs);
				Ast_Navigator::ast_handle(binop->rhs);

				auto binop_type = get_bytecode_from_binop(binop->binary_op);
	            auto bytecode_type = bytecode_get_type(binop->lhs->inferred_type);
	            INST(binop, Binary, binop_type, binop->reg, binop->lhs->reg, binop->rhs->reg, bytecode_type);

				break;
			}
		}
	}

	void handle (Ast_Literal* lit) {
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
				lit->data_offset = this->constants->add(lit->string_value);
	            INST(lit, Constant_Offset, lit->reg, lit->data_offset);
				break;
			}
			default: Logger::error(lit, "Literal type to bytecode conversion not supported!");
		}
	}

	void handle (Ast_Ident* ident) {
		if (ident->declaration->is_spilled) {
			if (this->current_scope->is_global()) {
				INST(ident, Global_Offset, ident->reg, ident->declaration->bytecode_stack_offset);
			} else {
				INST(ident, Stack_Offset, ident->reg, ident->declaration->bytecode_stack_offset);
			}

			if (!this->is_left_value && ident->inferred_type->can_be_in_register(INTERP_REGISTER_SIZE)) {
				INST(ident, Load, ident->reg, ident->reg, ident->inferred_type->byte_size);
			}
		}
	}

    void handle (Ast_Function* func) {
		if (func->is_native()) {
			INST(func, Set, func->reg, BYTECODE_TYPE_POINTER, &func->foreign_function_pointer);
		} else {
			INST(func, Set, func->reg, BYTECODE_TYPE_POINTER, &func);
		}
	}

	void handle (Ast_Function_Call* call) {
		Ast_Navigator::ast_handle(call->arguments);

		// TODO: move this to its own handler function
		INST(call, Call_Setup, DC_CALL_C_X64_WIN64, (uint8_t) call->arguments->unnamed.size());

		auto bytecode_type = BYTECODE_TYPE_VOID;
		for (uint8_t i = 0; i < call->arguments->unnamed.size(); i++) {
			auto exp = call->arguments->unnamed[i];

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
			Ast_Navigator::ast_handle(call->func);
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

	void print_extra_metrics() {
		print_extra_metric("Instructions created", "%zd", this->instruction_count);
	}
};
