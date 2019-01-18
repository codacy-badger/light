#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"

#include "ast/types.hpp"
#include "ast/type_conversion.hpp"

#include "util/logger.hpp"

#include <assert.h>

struct Type_Checking : Phase, Ast_Navigator {
    Arch* target_arch;

    Type_Checking() : Phase("Type Checking", CE_MODULE_CHECK_TYPES, true) { /* empty */ }

    // @Info this pipe ensures that all expressions have a valid inferred_type
	// and that all types in the tree make sense (binary ops, func calls, etc.).

    void on_start (Compiler_Settings* settings) {
        this->target_arch = settings->target_arch;
    }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

    void match_types (Ast_Expression** exp_ptr, Ast_Type_Instance* given, Ast_Type_Instance* expected) {
        std::vector<Ast_Statement*> to_prepend;
        if (!this->check_type_match(&to_prepend, exp_ptr, given, expected)) {
            Logger::error_and_stop(*exp_ptr, "Type mismatch: given '%s' but '%s' was expected",
                given->name, expected->name);
        } else this->prepend_statements(&to_prepend);
    }

    void match_types (Ast_Expression** exp_ptr1, Ast_Expression** exp_ptr2) {
        std::vector<Ast_Statement*> to_prepend;
        if (!this->check_type_match(&to_prepend, exp_ptr1, (*exp_ptr2)->inferred_type)
                && !this->check_type_match(&to_prepend, exp_ptr2, (*exp_ptr1)->inferred_type)) {
            Logger::error_and_stop(*exp_ptr1, "Type mismatch: '%s' and '%s' are not compatible",
                (*exp_ptr1)->inferred_type->name, (*exp_ptr2)->inferred_type->name);
        } else this->prepend_statements(&to_prepend);
    }

    void match_types (Ast_Expression** exp_ptr, Ast_Type_Instance* expected) {
        this->match_types(exp_ptr, (*exp_ptr)->inferred_type, expected);
    }

    bool check_type_match (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr, Ast_Type_Instance* expected) {
        return this->check_type_match(to_prepend, exp_ptr, (*exp_ptr)->inferred_type, expected);
    }

    bool check_type_match (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr, Ast_Type_Instance* given, Ast_Type_Instance* expected) {
        return (given == expected) || Type_Conversion::try_convert(to_prepend, exp_ptr, given, expected);
    }

	void ast_handle (Ast_Declaration* decl) {
		if (decl->expression) {
			this->ast_handle(decl->expression);

			if (decl->type) {
				auto decl_type_inst = static_cast<Ast_Type_Instance*>(decl->type);
                this->match_types(&decl->expression, decl_type_inst);
			}
		}

		Ast_Navigator::ast_handle(decl->type);

		assert(decl->type != NULL);
		assert(decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
	}

	void ast_handle (Ast_If* _if) {
		Ast_Navigator::ast_handle(_if);

        this->match_types(&_if->condition, Types::type_bool);
	}

	void ast_handle (Ast_While* _while) {
		Ast_Navigator::ast_handle(_while);

        this->match_types(&_while->condition, Types::type_bool);
	}

	void ast_handle (Ast_Return* ret) {
		auto fn = this->current_scope->get_parent_function();
		if (!fn) Logger::error_and_stop(ret, "Return statement must be inside a function");

		auto ret_type_def = static_cast<Ast_Type_Instance*>(fn->type->ret_type);
		if (ret->expression) {
			this->ast_handle(ret->expression);

			if (fn->type->ret_type == Types::type_void) {
				Logger::error_and_stop(ret, "Return statment has expression, but function returns void");
			}

            this->match_types(&ret->expression, ret_type_def);
		} else {
			if (fn->type->ret_type != Types::type_void)
				Logger::error_and_stop(ret, "Return statment has no expression, but function returns '%s'",
					ret_type_def->name);
		}
	}

    void ast_handle (Ast_Expression* exp) {
        if (exp->inferred_type) {
            Ast_Navigator::ast_handle(exp->inferred_type);
        }
        Ast_Navigator::ast_handle(exp);
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
		ptr->byte_size = this->target_arch->register_size;
		Ast_Navigator::ast_handle(ptr);
	}

	void ast_handle (Ast_Function_Type* fn) {
		fn->byte_size = this->target_arch->register_size;
		Ast_Navigator::ast_handle(fn);
	}

	void ast_handle (Ast_Array_Type* arr) {
		Ast_Navigator::ast_handle(arr);

		if (arr->length->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(arr->length);
			if (lit->literal_type == AST_LITERAL_UNSIGNED_INT) {
				arr->length_uint = lit->uint_value;
			} else Logger::error_and_stop(arr, "Arrays size must be an unsigned integer");
		} else Logger::error_and_stop(arr, "Arrays can only have constant size");

		arr->byte_size = this->target_arch->register_size;
	}

	void ast_handle (Ast_Function_Call* call) {
	    if (call->func->inferred_type->typedef_type != AST_TYPEDEF_FUNCTION) {
			Logger::error_and_stop(call, "Function calls can only be performed with functions types");
		}

		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

		for (int i = 0; i < call->arguments->unnamed.size(); i++) {
			if (i >= func_type->arg_decls.size()) break;

			this->ast_handle(call->arguments->unnamed[i]);
			auto param_exp = call->arguments->unnamed[i];
			assert(param_exp->inferred_type);

			auto arg_type = static_cast<Ast_Type_Instance*>(func_type->arg_decls[i]->type);
            this->match_types(&call->arguments->unnamed[i], arg_type);
		}

		for (auto entry : call->arguments->named) {
			this->ast_handle(entry.second);

			auto decl = func_type->get_declaration(entry.first);
			auto arg_type = static_cast<Ast_Type_Instance*>(decl->type);
            this->match_types(&entry.second, arg_type);
		}
	}

	void ast_handle (Ast_Binary* binop) {
	    Ast_Navigator::ast_handle(binop);

        if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
            if (binop->rhs->exp_type != AST_EXPRESSION_IDENT) {
                Logger::error_and_stop(binop, "Attribute name must be an identifier");
            }
        } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
            this->match_types(&binop->rhs, Types::type_u64);

            auto left_type = binop->lhs->inferred_type->typedef_type;
            if (left_type != AST_TYPEDEF_ARRAY
                    && left_type != AST_TYPEDEF_STRUCT
                    && left_type != AST_TYPEDEF_POINTER) {
                Logger::error_and_stop(binop, "Left of subscript operator is not array, slice or pointer type");
            }
		} else if (binop->binary_op == AST_BINARY_ASSIGN) {
            this->match_types(&binop->rhs, binop->lhs->inferred_type);
		} else {
            // @INFO Types don't match, but maybe we can add an implicid cast
            // to prevent dumb casts: u8 -> u32, s16 -> s64, etc...
            this->match_types(&binop->lhs, &binop->rhs);
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
                    auto literal = static_cast<Ast_Literal*>(unop->exp);
                    if (literal->literal_type != AST_LITERAL_STRING) {
                        Logger::error_and_stop(unop, "Literal value cannot be referenced");
                    }
	            }
	            break;
			}
		}
	}
};
