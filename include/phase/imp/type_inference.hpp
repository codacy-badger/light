#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"
#include "ast/ast_factory.hpp"

#include "util/logger.hpp"

struct Type_Inference : Phase, Ast_Navigator {
    Type_Inference() : Phase("Type Inference", CE_MODULE_INFER_TYPES) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

	void ast_handle (Ast_Declaration* decl) {
		if (!decl->expression && !decl->type) {
			Logger::error_and_stop(decl, "Cannot infer type without an expression");
		}

        // @TODO move this check to the parser, since we have the information
		if (decl->is_constant && !decl->expression) {
			Logger::error_and_stop(decl, "All constant declarations must have a value");
		}

        Ast_Navigator::ast_handle(decl);

        if (!decl->type) {
            decl->type = decl->expression->inferred_type;
        }

        // @TODO move this to a more convenient place, it makes no sense here
        if (decl->is_constant) {
            if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
                auto fn = static_cast<Ast_Function*>(decl->expression);
                fn->name = decl->name;
            } else if (decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                auto defn_ty = static_cast<Ast_Type*>(decl->expression);
                if (defn_ty->typedef_type == AST_TYPEDEF_STRUCT) {
                    auto _struct = static_cast<Ast_Struct_Type*>(defn_ty);
                    _struct->name = decl->name;
                }
            }
        }
	}

	void ast_handle (Ast_Type* type_inst) {
        Ast_Navigator::ast_handle(type_inst);

        if (!type_inst->inferred_type) {
            type_inst->inferred_type = Types::type_type;
        }
	}

	void ast_handle (Ast_Function_Type* func_type) {
        Ast_Navigator::ast_handle(func_type);

        if (!func_type->inferred_type) {
            func_type->inferred_type = Types::type_type;
        }
	}

	void ast_handle (Ast_Cast* cast) {
        Ast_Navigator::ast_handle(cast);

        if (cast->cast_to->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
            cast->inferred_type = static_cast<Ast_Type*>(cast->cast_to);
        } else Logger::error_and_stop(cast, "Cast target must be a type");
	}

	void ast_handle (Ast_Run* run) {
        Ast_Navigator::ast_handle(run);

        if (!run->inferred_type) {
            run->inferred_type = run->expression->inferred_type;
        }
	}

	void ast_handle (Ast_Function* func) {
        Ast_Navigator::ast_handle(func);

        if (!func->inferred_type) {
            func->inferred_type = func->type;
        }
	}

	void ast_handle (Ast_Function_Call* call) {
        Ast_Navigator::ast_handle(call);

        if (!call->inferred_type) {
            if (call->func->exp_type == AST_EXPRESSION_FUNCTION) {
                auto func = static_cast<Ast_Function*>(call->func);
                if (func->type->ret_type->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                    call->inferred_type = static_cast<Ast_Type*>(func->type->ret_type);
                }
            } else if (call->func->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(call->func);
                auto decl = ident->declaration;
                if (decl && decl->type && decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                    auto decl_type = static_cast<Ast_Type*>(decl->type);
                    if (decl_type->typedef_type == AST_TYPEDEF_FUNCTION) {
                        auto func_type = static_cast<Ast_Function_Type*>(decl_type);
                        if (func_type->ret_type->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                            call->inferred_type = static_cast<Ast_Type*>(func_type->ret_type);
                        }
                    }
                }
            }
        }
	}

	void ast_handle (Ast_Unary* unop) {
		Ast_Navigator::ast_handle(unop);

        if (!unop->inferred_type) {
    		switch (unop->unary_op) {
    			case AST_UNARY_NEGATE: {
    				unop->inferred_type = ast_get_container_signed(unop->exp->inferred_type);
    	            break;
    			}
    			case AST_UNARY_NOT: {
    				unop->inferred_type = Types::type_bool;
    				break;
    			}
    			case AST_UNARY_DEREFERENCE: {
    	            auto inf_type = unop->exp->inferred_type;
    	            if (inf_type->typedef_type == AST_TYPEDEF_POINTER) {
    	                auto ptr_type = static_cast<Ast_Pointer_Type*>(inf_type);
    	                auto base_type = static_cast<Ast_Type*>(ptr_type->base);
    	                unop->inferred_type = base_type;
    	            }
    	            break;
    			}
    			case AST_UNARY_REFERENCE: {
    			    unop->inferred_type = Ast_Factory::pointer_type(unop->exp->inferred_type);
                    unop->inferred_type->location = unop->exp->location;
    	            break;
    			}
    		}
        }
	}

	void ast_handle (Ast_Binary* binop) {
        Ast_Navigator::ast_handle(binop);

        if (!binop->inferred_type) {
            switch (binop->binary_op) {
                case AST_BINARY_ATTRIBUTE: {
                    auto type_def = binop->lhs->inferred_type;

                    if (binop->rhs->exp_type != AST_EXPRESSION_IDENT) {
                        Logger::error_and_stop(binop, "Expected identifier after attribute access");
                    }

                    if (type_def->typedef_type == AST_TYPEDEF_POINTER) {
                        auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
                        type_def = ptr_type->get_base_type_recursive();
                    }

                    if (type_def->typedef_type == AST_TYPEDEF_STRUCT) {
                        auto _struct = static_cast<Ast_Struct_Type*>(type_def);

                        auto ident = static_cast<Ast_Ident*>(binop->rhs);
                        auto attribute = _struct->find_attribute(ident->name);
                        if (attribute) {
                            auto attr_type = static_cast<Ast_Type*>(attribute->type);
                            binop->inferred_type = attr_type;
                            ident->inferred_type = attr_type;
                            ident->declaration = attribute;
                        } else Logger::error_and_stop(binop, "The type '%s' has no attribute named '%s'", _struct->name, ident->name);
                    } else if (type_def->typedef_type == AST_TYPEDEF_ARRAY) {
                        auto _array = static_cast<Ast_Array_Type*>(type_def);
                        auto ident = static_cast<Ast_Ident*>(binop->rhs);
                        if (strcmp(ident->name, "length") == 0) {
                            binop->inferred_type = Types::type_u64;
                        } else if (strcmp(ident->name, "data") == 0) {
                            binop->inferred_type = Ast_Factory::pointer_type(_array->base);
                            binop->inferred_type->location = _array->base->location;
                        } else Logger::error_and_stop(binop->rhs, "'%s' is not a valid attribute for array (use length or data)", ident->name);
                    }
                    break;
                }
                case AST_BINARY_SUBSCRIPT: {
                    if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
                        auto arr_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
                        binop->inferred_type = static_cast<Ast_Type*>(arr_type->base);
                    } else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
                        // @TODO refactor this once we have subscript operator overloading
                        auto _struct = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
                        if (_struct->is_slice) {
                            Ast_Navigator::ast_handle(binop->rhs);

                            auto data_decl = _struct->find_attribute("data");
                            auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
                            binop->inferred_type = static_cast<Ast_Type*>(ptr_type->base);
                        }
                    } else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_POINTER) {
                        auto ptr_type = static_cast<Ast_Pointer_Type*>(binop->lhs->inferred_type);
                        binop->inferred_type = static_cast<Ast_Type*>(ptr_type->base);
                    }
                    break;
                }
                case AST_BINARY_ASSIGN: {
                    Ast_Navigator::ast_handle(binop->rhs);
                    binop->inferred_type = Types::type_void;
                    break;
                }
                default: {
                    Ast_Navigator::ast_handle(binop->rhs);
                    binop->inferred_type = binop->get_result_type();
                    break;
                }
            }
        }
	}

	void ast_handle (Ast_Ident* ident) {
		if (!ident->inferred_type && ident->declaration) {
			ident->inferred_type = static_cast<Ast_Type*>(ident->declaration->type);
		}
	}

	void ast_handle (Ast_Literal* lit) {
		if (!lit->inferred_type) {
			switch (lit->literal_type) {
		        case AST_LITERAL_UNSIGNED_INT: {
					lit->inferred_type = ast_get_smallest_type(lit->uint_value);
		            break;
		        }
		        case AST_LITERAL_SIGNED_INT: {
					lit->inferred_type = ast_get_smallest_type(lit->int_value);
		            break;
		        }
		        case AST_LITERAL_DECIMAL: {
					lit->inferred_type = Types::type_f64;
		            break;
		        }
		        case AST_LITERAL_STRING: {
					lit->inferred_type = Types::type_string;
		            break;
		        }
		        default: Logger::internal(lit, "Unknown literal type: %d", lit->literal_type);
		    }
		}
	}
};
