#pragma once

#include "parser/pipes.hpp"

#include <assert.h>

#include "compiler.hpp"
#include "parser/ast.hpp"

#define WARN_MAX_DEREF_COUNT 3

#define ERROR(node, ...) report_error_and_stop(&node->location, __VA_ARGS__)
#define WARN(node, ...) report_warning(&node->location, __VA_ARGS__)

bool cast_if_possible (Ast_Expression** exp_ptr, Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
	if (ast_types_are_equal(type_from, type_to)) return true;
	else if (g_compiler->types->is_implicid_cast(type_from, type_to)) {
        auto cast = new Ast_Cast();
		cast->location = (*exp_ptr)->location;
        cast->value = (*exp_ptr);
        cast->cast_to = type_to;
        cast->inferred_type = type_to;
        (*exp_ptr) = cast;
        return true;
    } else return false;
}

void replace_ident_by_const (Ast_Ident** ident_ptr) {
	auto decl = (*ident_ptr)->declaration;
    auto _addr = reinterpret_cast<Ast_Expression**>(ident_ptr);
    delete *_addr;
    (*_addr) = decl->expression;
}

struct Type_Checking : Pipe {
	PIPE_NAME(Type_Checking)

	void handle (Ast_Import** import_ptr) {
		auto import = (*import_ptr);

		if (import->target->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(import->target);
			if (lit->literal_type == AST_LITERAL_STRING) {
				return;
			} else ERROR(import, "Import must be followed by a string literal");
		} else ERROR(import, "Import must be followed by an expression");
	}

	void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		if (decl->expression) {
			Pipe::handle(&decl->expression);

			if (!decl->expression->inferred_type) {
				ERROR(decl->expression, "Expression type could not be inferred");
			}

			if (decl->type) {
				auto decl_type_inst = static_cast<Ast_Type_Instance*>(decl->type);
				if (!ast_types_are_equal(decl_type_inst, decl->expression->inferred_type)) {
					if (!cast_if_possible(&decl->expression, decl->expression->inferred_type, decl_type_inst)) {
						ERROR(decl, "Type mismatch on declaration: value is '%s' but declaration wants '%s'",
							decl->expression->inferred_type->name, decl_type_inst->name);
					}
				}
			} else decl->type = decl->expression->inferred_type;

			Pipe::handle(&decl->type);
		} else if (decl->type) {
			Pipe::handle(&decl->type);
		} else ERROR(decl, "Cannot infer type without an expression");

	    if (!decl->type) {
	        ERROR(decl, "Type could not be inferred");
	    } else if (decl->type->inferred_type != g_compiler->type_def_type) {
			ERROR(decl, "Expression is not a type");
		}
	}

	void handle (Ast_If** if_ptr) {
		auto _if = (*if_ptr);

		Pipe::handle(if_ptr);

		if (!cast_if_possible(&_if->condition, _if->condition->inferred_type, g_compiler->type_def_bool)) {
			ERROR(_if, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_if->condition->inferred_type->name);
		}
	}

	void handle (Ast_While** while_ptr) {
		auto _while = (*while_ptr);

		Pipe::handle(while_ptr);

		if (!cast_if_possible(&_while->condition, _while->condition->inferred_type, g_compiler->type_def_bool)) {
			ERROR(_while, "The condition for the IF statement must be of type boolean, but it is '%s'",
				_while->condition->inferred_type->name);
		}
	}

	void handle (Ast_Return** ret_ptr) {
		auto ret = (*ret_ptr);

		if (ret->exp) Pipe::handle(&ret->exp);

		auto fn = ret->block->get_function();
		auto ret_type_def = static_cast<Ast_Type_Instance*>(fn->ret_type);
		if (!fn) {
			ERROR(ret, "Return statement must be inside a function");
		} else if (ret->exp) {
			if (fn->ret_type == g_compiler->type_def_void)
				ERROR(ret, "Return statment has expression, but function returns void");
			else if (ret->exp->inferred_type != fn->ret_type) {
	            if (!cast_if_possible(&ret->exp, ret->exp->inferred_type, ret_type_def)) {
	    			ERROR(ret, "Type mismatch, return expression is '%s', but function expects '%s'!",
	    				ret->exp->inferred_type->name, ret_type_def->name);
	            }
			}
		} else {
			if (fn->ret_type != g_compiler->type_def_void)
				ERROR(ret, "Return statment has no expression, but function returns '%s'!",
					ret_type_def->name);
		}
	}

	void handle (Ast_Expression** exp_ptr) {
		if (!(*exp_ptr)->inferred_type) {
			Pipe::handle(exp_ptr);
		}
	}

	void handle (Ast_Cast** cast_ptr) {
		auto cast = (*cast_ptr);

		Pipe::handle(&cast->value);
		Pipe::handle(&cast->cast_to);

		if (cast->cast_to->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
			cast->inferred_type = static_cast<Ast_Type_Instance*>(cast->cast_to);
		} else ERROR(cast, "Cast target is not a type");
	}

	void handle (Ast_Type_Instance** type_inst_ptr) {
		auto type_inst = (*type_inst_ptr);

		Pipe::handle(type_inst_ptr);

		if (type_inst && type_inst->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
			g_compiler->types->add_type_if_new(type_inst);
		} else abort();
	}

	void handle (Ast_Function_Type** func_type_ptr) {
		auto func_type = (*func_type_ptr);

	    func_type->inferred_type = g_compiler->type_def_type;

		Pipe::handle(&func_type->ret_type);
		for (int i = 0; i < func_type->arg_types.size(); i++) {
			Pipe::handle(&func_type->arg_types[i]);
		}
	}

	void handle (Ast_Struct_Type** _struct_ptr) {
		auto _struct = (*_struct_ptr);

	    _struct->inferred_type = g_compiler->type_def_type;
		for (auto &decl : _struct->attributes) {
			Pipe::handle(&decl);
		}

		if (_struct->byte_size == 0) {
			size_t byte_offset = 0;
			for (size_t i = 0; i < _struct->attributes.size(); i++) {
				auto decl = _struct->attributes[i];
				decl->attribute_byte_offset = byte_offset;
				decl->attribute_index = i;

				assert (decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
				auto defn_ty = static_cast<Ast_Type_Instance*>(decl->type);
				byte_offset += defn_ty->byte_size;
			}
			_struct->byte_size = byte_offset;
		}
	}

	void handle (Ast_Array_Type** arr_ptr) {
		auto arr = (*arr_ptr);

	    arr->inferred_type = g_compiler->type_def_type;
		Pipe::handle(&arr->base);
		Pipe::handle(&arr->length);

		if (arr->length->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = static_cast<Ast_Literal*>(arr->length);
			if (lit->literal_type != AST_LITERAL_UNSIGNED_INT) {
				ERROR(arr, "Arrays size must be an unsigned integer");
			}
		} else ERROR(arr, "Arrays can only have constant size");

		auto type_def = static_cast<Ast_Type_Instance*>(arr->base);
		arr->byte_size = arr->get_length() * type_def->byte_size;
	}

	void handle (Ast_Pointer_Type** ptr_type_ptr) {
		auto ptr_type = (*ptr_type_ptr);

	    ptr_type->inferred_type = g_compiler->type_def_type;
		Pipe::handle(&ptr_type->base);
		if (ptr_type->base->exp_type == AST_EXPRESSION_IDENT) {
			auto ident = static_cast<Ast_Ident*>(ptr_type->base);
			delete ptr_type->base;
			ptr_type->base = ident->declaration->expression;
		}
	}

	void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

		if (func->inferred_type == NULL) {
			for (auto &arg_decl : func->arg_decls) {
				Pipe::handle(&arg_decl);
			}
			Pipe::handle(&func->ret_type);
		    func->inferred_type = g_compiler->types->build_function_type(func);
			Pipe::handle(&func->inferred_type);
			if (func->scope) Pipe::handle(&func->scope);
		}
	}

	void handle (Ast_Function_Call** call_ptr) {
		auto call = (*call_ptr);

		Pipe::handle(&call->func);

	    if (call->func->inferred_type->typedef_type != AST_TYPEDEF_FUNCTION)
			ERROR(call, "Function calls can only be performed to functions types");

		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);
		auto ret_ty = static_cast<Ast_Type_Instance*>(func_type->ret_type);
		call->inferred_type = ret_ty;

		if (call->arguments.size() == func_type->arg_types.size()) {
			for (int i = 0; i < call->arguments.size(); i++) {
				Pipe::handle(&call->arguments[i]);

				auto arg_type = static_cast<Ast_Type_Instance*>(func_type->arg_types[i]);
				auto param_exp = call->arguments[i];

				if (!cast_if_possible(&call->arguments[i], param_exp->inferred_type, arg_type)) {
					ERROR(call, "Type mismatch on parameter %d, expected '%s' but got '%s'",
						i + 1, arg_type->name, param_exp->inferred_type->name);
				}
			}
		} else {
			ERROR(call, "Wrong number of arguments, function has %d, but call has %d",
				func_type->arg_types.size(), call->arguments.size());
		}
	}

	void handle (Ast_Binary** binop_ptr) {
		auto binop = (*binop_ptr);

	    Pipe::handle(&binop->lhs);
	    if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
			auto type_def = binop->lhs->inferred_type;

			// TODO: precompute depth for each pointer type (when uniqued?)
			uint8_t deref_count = 0;
			while (type_def->typedef_type == AST_TYPEDEF_POINTER) {
				auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
				type_def = static_cast<Ast_Type_Instance*>(ptr_type->base);
				deref_count += 1;
			}
			if (deref_count > WARN_MAX_DEREF_COUNT) {
				WARN(binop, "Attribute access on deep pointer (%d)", deref_count);
			}

			if (type_def->typedef_type == AST_TYPEDEF_STRUCT) {
	            auto _struct = static_cast<Ast_Struct_Type*>(type_def);
	            if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
	                auto ident = static_cast<Ast_Ident*>(binop->rhs);
	                auto attribute = _struct->find_attribute(ident->name);
	                if (attribute) {
	                    auto attr_type = static_cast<Ast_Type_Instance*>(attribute->type);
	                    ident->inferred_type = attr_type;
	                    binop->inferred_type = attr_type;
	                    ident->declaration = attribute;
	                } else ERROR(binop, "The type '%s' has no attribute named '%s'", _struct->name, ident->name);
	            } else ERROR(binop, "Right of attribute access is NOT an identifier");
	        } else if (type_def->typedef_type == AST_TYPEDEF_ARRAY) {
				auto _array = static_cast<Ast_Array_Type*>(type_def);
				if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
					auto ident = static_cast<Ast_Ident*>(binop->rhs);
					if (strcmp(ident->name, "length") == 0) {
						binop->inferred_type = g_compiler->type_def_u64;
					} else if (strcmp(ident->name, "data") == 0) {
						binop->inferred_type = g_compiler->types->get_pointer_type(_array->base);
					} else ERROR(binop->rhs, "'%s' is not a valid attribute for array (use length or data)", ident->name);
				} else ERROR(binop, "Right of attribute access is NOT an identifier");
			} else ERROR(binop, "Left of attribute access has invalid type: '%s'", type_def->name);
	    } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
			if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
				auto arr_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
				binop->inferred_type = static_cast<Ast_Type_Instance*>(arr_type->base);

				Pipe::handle(&binop->rhs);
				if (!cast_if_possible(&binop->rhs, binop->rhs->inferred_type, g_compiler->type_def_u64)) {
					ERROR(binop, "Type '%s' cannot be casted to u64 (index)", binop->rhs->inferred_type->name);
				}
			} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
				auto _struct = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
				if (_struct->is_slice) {
					auto data_decl = _struct->find_attribute("data");
					if (data_decl) {
						Pipe::handle(&binop->rhs);
						auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
						binop->inferred_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
					} else ERROR(binop, "Slice type doesn't have data attribute");
				} else ERROR(binop, "Left struct is not a slice");
			} else ERROR(binop, "Left of array access is not of array or slice type");
		} else if (binop->binary_op == AST_BINARY_ASSIGN) {
			Pipe::handle(&binop->rhs);
			if (binop->lhs->inferred_type != binop->rhs->inferred_type) {
	            if (!cast_if_possible(&binop->rhs, binop->rhs->inferred_type, binop->lhs->inferred_type)) {
	                ERROR(binop, "Type mismatch on assign: from '%s' to '%s'",
	                    binop->rhs->inferred_type->name, binop->lhs->inferred_type->name);
	            }
	    	}
			binop->inferred_type = binop->rhs->inferred_type;
		} else {
	    	Pipe::handle(&binop->rhs);
	    	if (binop->lhs->inferred_type != binop->rhs->inferred_type) {
	            // Types don't match, but maybe we can add an implicid cast
	            // to prevent dumb casts: u8 -> u32, s16 -> s64, etc...
	            if (!cast_if_possible(&binop->lhs, binop->lhs->inferred_type, binop->rhs->inferred_type)) {
	                if (!cast_if_possible(&binop->rhs, binop->rhs->inferred_type, binop->lhs->inferred_type)) {
	                    ERROR(binop, "Type mismatch on binary expression: '%s' and '%s'",
	                        binop->lhs->inferred_type->name, binop->rhs->inferred_type->name);
	                }
	            }
	    	}
	    	switch (binop->binary_op) {
	    		case AST_BINARY_EQ:
	    		case AST_BINARY_NEQ:
	    		case AST_BINARY_LT:
	    		case AST_BINARY_LTE:
	    		case AST_BINARY_GT:
	    		case AST_BINARY_GTE: 	binop->inferred_type = g_compiler->type_def_bool; break;
	    		default: 				binop->inferred_type = binop->lhs->inferred_type; break;
	    	}
	    }
	}

	void handle (Ast_Unary** unop_ptr) {
		auto unop = (*unop_ptr);

		Pipe::handle(&unop->exp);
		switch (unop->unary_op) {
			case AST_UNARY_NEGATE: {
				if (unop->exp->inferred_type == g_compiler->type_def_u8) {
					unop->inferred_type = g_compiler->type_def_s16;
				} else if (unop->exp->inferred_type == g_compiler->type_def_u16) {
					unop->inferred_type = g_compiler->type_def_s32;
				} else if (unop->exp->inferred_type == g_compiler->type_def_u32) {
					unop->inferred_type = g_compiler->type_def_s64;
				} else if (unop->exp->inferred_type == g_compiler->type_def_u64) {
					unop->inferred_type = g_compiler->type_def_s64;
				} else {
					unop->inferred_type = unop->exp->inferred_type;
				}
	            break;
			}
			case AST_UNARY_NOT: {
				unop->inferred_type = g_compiler->type_def_bool;
				break;
			}
			case AST_UNARY_DEREFERENCE: {
	            auto inf_type = unop->exp->inferred_type;
	            if (inf_type->typedef_type == AST_TYPEDEF_POINTER) {
	                auto ptr_type = static_cast<Ast_Pointer_Type*>(inf_type);
	                auto base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
	                unop->inferred_type = base_type;
	            } else ERROR(unop, "Can't dereference a non-pointer type expression");
	            break;
			}
			case AST_UNARY_REFERENCE: {
			    unop->inferred_type = g_compiler->types->get_pointer_type(unop->exp->inferred_type);
				Pipe::handle(&unop->inferred_type);
	            break;
			}
		}
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (ident->declaration) {
			if (ident->declaration->is_constant()) {
	            replace_ident_by_const(ident_ptr);
				Pipe::handle(&ident->declaration);
	        } else {
				ident->inferred_type = static_cast<Ast_Type_Instance*>(ident->declaration->type);
			}
		} else ERROR(ident, "Indetifier '%s' has no declaration", ident->name);
	}

	void handle (Ast_Literal** lit_ptr) {
		auto lit = (*lit_ptr);

	    switch (lit->literal_type) {
	        case AST_LITERAL_UNSIGNED_INT: {
				if (lit->uint_value <= UINT32_MAX) {
					if (lit->uint_value <= UINT16_MAX) {
						if (lit->uint_value <= UINT8_MAX) {
							lit->inferred_type = g_compiler->type_def_u8;
						} else lit->inferred_type = g_compiler->type_def_u16;
					} else lit->inferred_type = g_compiler->type_def_u32;
				} else lit->inferred_type = g_compiler->type_def_u64;
	            break;
	        }
	        case AST_LITERAL_SIGNED_INT: {
				if (lit->int_value <= INT32_MAX && lit->int_value >= INT32_MIN) {
					if (lit->int_value <= INT16_MAX && lit->int_value >= INT16_MIN) {
						if (lit->int_value <= INT8_MAX && lit->int_value >= INT8_MIN) {
							lit->inferred_type = g_compiler->type_def_s8;
						} else lit->inferred_type = g_compiler->type_def_s16;
					} else lit->inferred_type = g_compiler->type_def_s32;
				} else lit->inferred_type = g_compiler->type_def_s64;
	            break;
	        }
	        case AST_LITERAL_DECIMAL: {
				lit->inferred_type = g_compiler->type_def_f32;
	            break;
	        }
	        case AST_LITERAL_STRING: {
				lit->inferred_type = g_compiler->types->get_pointer_type(g_compiler->type_def_u8);
				Pipe::handle(&lit->inferred_type);
	            break;
	        }
	        default: abort();
	    }
	}
};
