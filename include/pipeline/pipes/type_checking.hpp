#pragma once

#include "pipeline/pipe.hpp"

#include "compiler.hpp"
#include "ast/ast.hpp"

struct Type_Checking : Pipe {
	PIPE_NAME(Type_Checking)

	// @Info this pipe ensures that all expressions have a valid inferred_type
	// and that all types in the tree make sense (binary ops, func calls, etc.).
	// After this pipe has run, we can assume all inferred_types have value

	void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		if (!decl->expression && !decl->type) {
			ERROR_STOP(decl, "Cannot infer type without an expression");
		}

		if (decl->is_constant() && !decl->expression) {
			ERROR_STOP(decl, "All constant declarations must have a value");
		}

		if (decl->expression) {
			Pipe::handle(&decl->expression);

			if (decl->type) {
				auto decl_type_inst = static_cast<Ast_Type_Instance*>(decl->type);
				if (!try_cast(&decl->expression, decl_type_inst)) {
					ERROR_STOP(decl, "Type mismatch on declaration: value is '%s' but declaration wants '%s'",
						decl->expression->inferred_type->name, decl_type_inst->name);
				}
			} else decl->type = decl->expression->inferred_type;
		}

		Pipe::handle(&decl->type);

		ASSERT(decl->type != NULL);
		ASSERT(decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
	}

	void handle (Ast_If** if_ptr) {
		auto _if = (*if_ptr);

		Pipe::handle(if_ptr);

		if (!try_cast(&_if->condition, Types::type_bool)) {
			ERROR_STOP(_if, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_if->condition->inferred_type->name);
		}
	}

	void handle (Ast_While** while_ptr) {
		auto _while = (*while_ptr);

		Pipe::handle(while_ptr);

		if (!try_cast(&_while->condition, Types::type_bool)) {
			ERROR_STOP(_while, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_while->condition->inferred_type->name);
		}
	}

	void handle (Ast_Return** ret_ptr) {
		auto ret = (*ret_ptr);

		auto fn = ret->scope->get_parent_function();
		if (!fn) ERROR_STOP(ret, "Return statement must be inside a function");

		auto ret_type_def = static_cast<Ast_Type_Instance*>(fn->type->ret_type);
		if (ret->expression) {
			Pipe::handle(&ret->expression);

			if (fn->type->ret_type == Types::type_void) {
				ERROR_STOP(ret, "Return statment has expression, but function returns void");
			}

            if (!try_cast(&ret->expression, ret_type_def)) {
    			ERROR_STOP(ret, "Type mismatch, return expression is '%s', but function expects '%s'",
    				ret->expression->inferred_type->name, ret_type_def->name);
            }
		} else {
			if (fn->type->ret_type != Types::type_void)
				ERROR_STOP(ret, "Return statment has no expression, but function returns '%s'",
					ret_type_def->name);
		}
	}

	void handle (Ast_Expression** exp_ptr) {
		auto exp = (*exp_ptr);

		if (!exp->inferred_type) {
			Pipe::handle(exp_ptr);

			exp = (*exp_ptr);
			if (!exp->inferred_type) {
				ERROR_STOP(exp, "Expression type could not be inferred");
			}
		}
	}

	void handle (Ast_Cast** cast_ptr) {
		Pipe::handle(cast_ptr);

		auto cast = (*cast_ptr);
		if (cast->cast_to->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
			cast->inferred_type = static_cast<Ast_Type_Instance*>(cast->cast_to);
		} else ERROR_STOP(cast, "Cast target is not a type");
	}

	void handle (Ast_Type_Instance** type_inst_ptr) {
		auto type_inst = (*type_inst_ptr);

		type_inst->inferred_type = Types::type_type;
		Pipe::handle(type_inst_ptr);

		Compiler::instance->types->add_type_if_new(type_inst);
	}

	void handle (Ast_Struct_Type** _struct_ptr) {
		Pipe::handle(_struct_ptr);

		auto _struct = (*_struct_ptr);
		if (_struct->byte_size == 0) {
			// TODO: use byte_alignment to correctly assign byte_offsets
			for (auto attr : _struct->attributes) {
				auto attr_type = static_cast<Ast_Type_Instance*>(attr->type);

				attr->attribute_byte_offset = _struct->byte_size;
				_struct->byte_size += attr_type->byte_size;
			}
		}
	}

	void handle (Ast_Array_Type** arr_ptr) {
		auto arr = (*arr_ptr);

		Pipe::handle(&arr->base);
		Pipe::handle(&arr->length);

		if (arr->length->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(arr->length);
			if (lit->literal_type != AST_LITERAL_UNSIGNED_INT) {
				ERROR_STOP(arr, "Arrays size must be an unsigned integer");
			}
		} else ERROR_STOP(arr, "Arrays can only have constant size");

		auto type_def = static_cast<Ast_Type_Instance*>(arr->base);
		arr->byte_size = arr->get_length() * type_def->byte_size;
	}

	void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

		if (func->inferred_type == NULL) {
			Pipe::handle(&func->type);
		    func->inferred_type = func->type;

			if (func->scope) {
				Pipe::handle(&func->scope);
			}
		}
	}

	void handle (Ast_Function_Call** call_ptr) {
		auto call = (*call_ptr);

		Pipe::handle(&call->func);

	    if (call->func->inferred_type->typedef_type != AST_TYPEDEF_FUNCTION) {
			ERROR_STOP(call, "Function calls can only be performed to functions types");
		}

		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);
		auto ret_ty = static_cast<Ast_Type_Instance*>(func_type->ret_type);
		call->inferred_type = ret_ty;

		if (call->arguments.size() == func_type->arg_decls.size()) {
			for (int i = 0; i < call->arguments.size(); i++) {
				Pipe::handle(&call->arguments[i]);
				auto param_exp = call->arguments[i];
				ASSERT(param_exp->inferred_type);

				auto arg_type = static_cast<Ast_Type_Instance*>(func_type->arg_decls[i]->type);
				if (!try_cast(&call->arguments[i], arg_type)) {
					ERROR_STOP(call, "Type mismatch on parameter %d, expected '%s' but got '%s'",
						i + 1, arg_type->name, param_exp->inferred_type->name);
				}
			}
		} else {
			ERROR_STOP(call, "Wrong number of arguments, function has %d, but call has %d",
				func_type->arg_decls.size(), call->arguments.size());
		}
	}

	void handle (Ast_Binary** binop_ptr) {
		auto binop = (*binop_ptr);

	    Pipe::handle(&binop->lhs);
	    if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
			auto type_def = binop->lhs->inferred_type;
			if (type_def->typedef_type == AST_TYPEDEF_POINTER) {
				auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
				type_def = ptr_type->get_base_type_recursive();
			}

			if (type_def->typedef_type == AST_TYPEDEF_STRUCT) {
	            auto _struct = static_cast<Ast_Struct_Type*>(type_def);
	            if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
	                auto ident = static_cast<Ast_Ident*>(binop->rhs);
	                auto attribute = _struct->find_attribute(ident->name);
	                if (attribute) {
	                    auto attr_type = static_cast<Ast_Type_Instance*>(attribute->type);
	                    binop->inferred_type = attr_type;
	                    ident->declaration = attribute;
	                } else ERROR_STOP(binop, "The type '%s' has no attribute named '%s'", _struct->name, ident->name);
	            } else ERROR_STOP(binop, "Right of attribute access is NOT an identifier");
	        } else if (type_def->typedef_type == AST_TYPEDEF_ARRAY) {
				auto _array = static_cast<Ast_Array_Type*>(type_def);
				if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
					auto ident = static_cast<Ast_Ident*>(binop->rhs);
					if (strcmp(ident->name, "length") == 0) {
						binop->inferred_type = Types::type_u64;
					} else if (strcmp(ident->name, "data") == 0) {
						binop->inferred_type = Compiler::instance->types->get_pointer_type(_array->base);
					} else ERROR_STOP(binop->rhs, "'%s' is not a valid attribute for array (use length or data)", ident->name);
				} else ERROR_STOP(binop, "Right of attribute access is NOT an identifier");
			} else ERROR_STOP(binop, "Left of attribute access has invalid type: '%s'", type_def->name);
	    } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
			if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
				auto arr_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
				binop->inferred_type = static_cast<Ast_Type_Instance*>(arr_type->base);

				Pipe::handle(&binop->rhs);
				if (!try_cast(&binop->rhs, Types::type_u64)) {
					ERROR_STOP(binop, "Type '%s' cannot be casted to u64 (index)", binop->rhs->inferred_type->name);
				}
			} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
				// TODO: refactor this once we have subscript operator overloading
				auto _struct = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
				if (_struct->is_slice) {
					Pipe::handle(&binop->rhs);

					auto data_decl = _struct->find_attribute("data");
					auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
					binop->inferred_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
				} else ERROR_STOP(binop, "Left struct is not a slice");
			} else ERROR_STOP(binop, "Left of subscript is not of array or slice type");
		} else if (binop->binary_op == AST_BINARY_ASSIGN) {
			Pipe::handle(&binop->rhs);
            if (!try_cast(&binop->rhs, binop->lhs->inferred_type)) {
                ERROR_STOP(binop, "Type mismatch on assign: from '%s' to '%s'",
                    binop->rhs->inferred_type->name, binop->lhs->inferred_type->name);
            }
			binop->inferred_type = binop->rhs->inferred_type;
		} else {
	    	Pipe::handle(&binop->rhs);
            // Types don't match, but maybe we can add an implicid cast
            // to prevent dumb casts: u8 -> u32, s16 -> s64, etc...
            if (!try_cast(&binop->lhs, binop->rhs->inferred_type)) {
                if (!try_cast(&binop->rhs, binop->lhs->inferred_type)) {
                    ERROR_STOP(binop, "Type mismatch on binary expression: '%s' and '%s'",
                        binop->lhs->inferred_type->name, binop->rhs->inferred_type->name);
                }
            }
	    	binop->inferred_type = binop->get_result_type();
	    }
	}

	void handle (Ast_Unary** unop_ptr) {
		auto unop = (*unop_ptr);

		Pipe::handle(&unop->exp);
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
	                auto base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
	                unop->inferred_type = base_type;
	            } else ERROR_STOP(unop, "Can't dereference a non-pointer type expression");
	            break;
			}
			case AST_UNARY_REFERENCE: {
			    unop->inferred_type = Compiler::instance->types->get_pointer_type(unop->exp->inferred_type);
				Pipe::handle(&unop->inferred_type);
	            break;
			}
		}
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (ident->declaration) {
			if (ident->declaration->is_constant()) {
				Pipe::handle(&ident->declaration);

			    auto _addr = reinterpret_cast<Ast_Expression**>(ident_ptr);
			    (*_addr) = (*ident_ptr)->declaration->expression;
	        } else {
				ident->inferred_type = static_cast<Ast_Type_Instance*>(ident->declaration->type);
			}
		} else ERROR_STOP(ident, "Indetifier '%s' has no declaration", ident->name);
	}

	void handle (Ast_Literal** lit_ptr) {
		auto lit = (*lit_ptr);

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
					lit->inferred_type = Types::type_f32;
		            break;
		        }
		        case AST_LITERAL_STRING: {
					lit->inferred_type = Compiler::instance->types->get_pointer_type(Types::type_u8);
					Pipe::handle(&lit->inferred_type);
		            break;
		        }
		        default: INTERNAL(lit, "Unknown literal type: %d", lit->literal_type);
		    }
		}
	}
};
