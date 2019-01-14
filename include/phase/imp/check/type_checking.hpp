#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"

#include "util/logger.hpp"

struct Type_Checking : Async_Phase, Ast_Navigator {
    Type_Checking() : Async_Phase("Type Checking", CE_MODULE_CHECK_TYPES) { /* empty */ }

    // @Info this pipe ensures that all expressions have a valid inferred_type
	// and that all types in the tree make sense (binary ops, func calls, etc.).

    void handle_main_event (void* data) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

	void ast_handle (Ast_Declaration* decl) {
		if (decl->expression) {
			Ast_Navigator::ast_handle(decl->expression);

			if (decl->type) {
				auto decl_type_inst = static_cast<Ast_Type_Instance*>(decl->type);
				if (!try_cast(&decl->expression, decl_type_inst)) {
					Logger::error_and_stop(decl, "Type mismatch on declaration: value is '%s' but declaration wants '%s'",
						decl->expression->inferred_type->name, decl_type_inst->name);
				}
			}
		}

		Ast_Navigator::ast_handle(decl->type);

		assert(decl->type != NULL);
		assert(decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
	}

	void ast_handle (Ast_If* _if) {
		Ast_Navigator::ast_handle(_if);

		if (!try_cast(&_if->condition, Types::type_bool)) {
			Logger::error_and_stop(_if, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_if->condition->inferred_type->name);
		}
	}

	void ast_handle (Ast_While* _while) {
		Ast_Navigator::ast_handle(_while);

		if (!try_cast(&_while->condition, Types::type_bool)) {
			Logger::error_and_stop(_while, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_while->condition->inferred_type->name);
		}
	}

	void ast_handle (Ast_Return* ret) {
		auto fn = this->current_scope->get_parent_function();
		if (!fn) Logger::error_and_stop(ret, "Return statement must be inside a function");

		auto ret_type_def = static_cast<Ast_Type_Instance*>(fn->type->ret_type);
		if (ret->expression) {
			Ast_Navigator::ast_handle(ret->expression);

			if (fn->type->ret_type == Types::type_void) {
				Logger::error_and_stop(ret, "Return statment has expression, but function returns void");
			}

            if (!try_cast(&ret->expression, ret_type_def)) {
    			Logger::error_and_stop(ret, "Type mismatch, return expression is '%s', but function expects '%s'",
    				ret->expression->inferred_type->name, ret_type_def->name);
            }
		} else {
			if (fn->type->ret_type != Types::type_void)
				Logger::error_and_stop(ret, "Return statment has no expression, but function returns '%s'",
					ret_type_def->name);
		}
	}

	void ast_handle (Ast_Struct_Type* _struct) {
		Ast_Navigator::ast_handle(_struct);

		if (_struct->byte_size == 0) {
			for (auto attr : _struct->attributes) {
				auto attr_type = static_cast<Ast_Type_Instance*>(attr->type);

				auto over = _struct->byte_size % attr_type->byte_size;
				if (over > 0) {
					auto padding = attr_type->byte_size - over;
					_struct->byte_padding += padding;
					_struct->byte_size += padding;
				}

				attr->attribute_byte_offset = _struct->byte_size;
				_struct->byte_size += attr_type->byte_size;
			}
		}
	}

	void ast_handle (Ast_Pointer_Type* ptr) {
		ptr->byte_size = this->settings->target_arch->register_size;

		Ast_Navigator::ast_handle(ptr);
	}

	void ast_handle (Ast_Array_Type* arr) {
		Ast_Navigator::ast_handle(arr->base);
		Ast_Navigator::ast_handle(arr->length);

		if (arr->length->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(arr->length);
			if (lit->literal_type != AST_LITERAL_UNSIGNED_INT) {
				Logger::error_and_stop(arr, "Arrays size must be an unsigned integer");
			}
		} else Logger::error_and_stop(arr, "Arrays can only have constant size");

		auto type_def = static_cast<Ast_Type_Instance*>(arr->base);
		arr->byte_size = arr->get_length() * type_def->byte_size;
	}

	void ast_handle (Ast_Function_Call* call) {
	    if (call->func->inferred_type->typedef_type != AST_TYPEDEF_FUNCTION) {
			Logger::error_and_stop(call, "Function calls can only be performed with functions types");
		}

		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

		for (int i = 0; i < call->arguments->unnamed.size(); i++) {
			if (i >= func_type->arg_decls.size()) break;

			Ast_Navigator::ast_handle(call->arguments->unnamed[i]);
			auto param_exp = call->arguments->unnamed[i];
			assert(param_exp->inferred_type);

			auto arg_type = static_cast<Ast_Type_Instance*>(func_type->arg_decls[i]->type);
			if (!try_cast(&call->arguments->unnamed[i], arg_type)) {
				Logger::error_and_stop(call, "Type mismatch on parameter %d, expected '%s' but got '%s'",
					i + 1, arg_type->name, param_exp->inferred_type->name);
			}
		}

		for (auto entry : call->arguments->named) {
			Ast_Navigator::ast_handle(entry.second);

			auto decl = func_type->get_declaration(entry.first);
			auto arg_type = static_cast<Ast_Type_Instance*>(decl->type);
			if (!try_cast(&entry.second, arg_type)) {
				Logger::error_and_stop(call, "Type mismatch on named parameter '%s', expected '%s' but got '%s'",
					entry.first, arg_type->name, entry.second->inferred_type->name);
			}
		}
	}

	void ast_handle (Ast_Binary* binop) {
	    Ast_Navigator::ast_handle(binop);

        if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
            if (binop->rhs->exp_type != AST_EXPRESSION_IDENT) {
                Logger::error_and_stop(binop, "Attribute name must be an identifier");
            }
        } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
            if (!try_cast(&binop->rhs, Types::type_u64)) {
                Logger::error_and_stop(binop, "Type '%s' cannot be casted to u64 (index)", binop->rhs->inferred_type->name);
            }

            auto left_type = binop->lhs->inferred_type->typedef_type;
            if (left_type != AST_TYPEDEF_ARRAY
                    && left_type != AST_TYPEDEF_STRUCT
                    && left_type != AST_TYPEDEF_POINTER) {
                Logger::error_and_stop(binop, "Left of subscript operator is not array, slice or pointer type");
            }
		} else if (binop->binary_op == AST_BINARY_ASSIGN) {
            if (!try_cast(&binop->rhs, binop->lhs->inferred_type)) {
                Logger::error_and_stop(binop, "Type mismatch on assignment: '%s' -> '%s'",
                    binop->rhs->inferred_type->name, binop->lhs->inferred_type->name);
            }
		} else {
            // @INFO Types don't match, but maybe we can add an implicid cast
            // to prevent dumb casts: u8 -> u32, s16 -> s64, etc...
            if (!try_cast(&binop->lhs, binop->rhs->inferred_type)
                    && !try_cast(&binop->rhs, binop->lhs->inferred_type)) {
                Logger::error_and_stop(binop, "Type mismatch on binary expression: '%s' and '%s'",
                    binop->lhs->inferred_type->name, binop->rhs->inferred_type->name);
            }
	    	binop->inferred_type = binop->get_result_type();
	    }
	}

	void ast_handle (Ast_Unary* unop) {
		Ast_Navigator::ast_handle(unop);

		switch (unop->unary_op) {
			case AST_UNARY_NEGATE: {
                if (!unop->exp->inferred_type->is_number) {
                    Logger::error_and_stop(unop, "Only numeric type expressions can be negated (-)");
                }
	            break;
			}
			case AST_UNARY_NOT: {
                if (!unop->exp->inferred_type->is_primitive) {
                    Logger::error_and_stop(unop, "Only primitive type expressions can be negated (!)");
                }
				break;
			}
			case AST_UNARY_DEREFERENCE: {
	            if (unop->exp->inferred_type->typedef_type != AST_TYPEDEF_POINTER) {
                    Logger::error_and_stop(unop, "Can't dereference (&) a non-pointer type expression");
	            }
	            break;
			}
			case AST_UNARY_REFERENCE: {
	            if (unop->exp->exp_type == AST_EXPRESSION_LITERAL) {
                    Logger::error_and_stop(unop, "Literal expression cannot be referenced (*)");
	            }
	            break;
			}
		}
	}
};
