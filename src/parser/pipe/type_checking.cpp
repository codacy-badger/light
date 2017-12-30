#include "parser/pipe/type_checking.hpp"

#include "compiler.hpp"
#include "parser/ast.hpp"

#define ERROR(node, ...) report_error_stop(&node->location, __VA_ARGS__)

bool cast_if_possible (Ast_Expression** exp_ptr, Ast_Type_Definition* type_from, Ast_Type_Definition* type_to) {
	if (type_from == type_to) return true;
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

void replace_slice_type (Ast_Array_Type** array_type_ptr) {
	auto slice_type = ast_make_slice_type(*array_type_ptr);
	slice_type->location = (*array_type_ptr)->location;
	delete (*array_type_ptr);
	(*array_type_ptr) = reinterpret_cast<Ast_Array_Type*>(slice_type);
}

void Type_Checking::on_statement(Ast_Statement* stm) {
    this->check_type(stm);
    this->to_next(stm);
}

void Type_Checking::check_type (Ast_Statement* stm) {
    for (auto note : stm->notes) check_type(note);
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK:
            check_type(static_cast<Ast_Block*>(stm));
            break;
        case AST_STATEMENT_IF:
            check_type(static_cast<Ast_If*>(stm));
            break;
        case AST_STATEMENT_WHILE:
            check_type(static_cast<Ast_While*>(stm));
            break;
        case AST_STATEMENT_DECLARATION:
            check_type(static_cast<Ast_Declaration*>(stm));
            break;
        case AST_STATEMENT_RETURN:
            check_type(static_cast<Ast_Return*>(stm));
            break;
        case AST_STATEMENT_EXPRESSION:
            check_type(static_cast<Ast_Expression*>(stm));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Note* note) {
	for (auto exp : note->arguments)
		check_type(exp);
}

void Type_Checking::check_type (Ast_Block* block) {
    for (auto stm : block->list) check_type(stm);
}

void Type_Checking::check_type (Ast_If* _if) {
	check_type(_if->condition);
	check_type(_if->then_statement);
	if (_if->else_statement) check_type(_if->else_statement);
}

void Type_Checking::check_type (Ast_While* _while) {
	check_type(_while->condition);
	check_type(_while->statement);
}

void Type_Checking::check_type (Ast_Declaration* decl) {
	if (decl->type) check_type(decl->type);

	if (decl->expression) {
		check_type(decl->expression);
		if (!decl->expression->inferred_type) {
			ERROR(decl->expression, "Expression type could not be inferred!");
		}

		if (!decl->type) {
			decl->type = decl->expression->inferred_type;
		}
	} else {
		if (!decl->type) {
			ERROR(decl, "Cannot infer type without an expression!");
		}
	}

    if (!decl->type) {
        ERROR(decl, "Type could not be inferred!");
    } else if (decl->type->inferred_type != g_compiler->type_def_type) {
		ERROR(decl, "Expression is not a type!");
	}
}

void Type_Checking::check_type (Ast_Return* ret) {
	if (ret->exp) check_type(ret->exp);

	auto fn = ret->block->get_function();
	auto ret_type_def = static_cast<Ast_Type_Definition*>(fn->type->return_type);
	if (!fn) {
		ERROR(ret, "Return statement must be inside a function!");
	} else if (ret->exp) {
		if (fn->type->return_type == g_compiler->type_def_void)
			ERROR(ret, "Return statment has expression, but function returns void!");
		else if (ret->exp->inferred_type != fn->type->return_type) {
            if (!cast_if_possible(&ret->exp, ret->exp->inferred_type, ret_type_def)) {
    			ERROR(ret, "Type mismatch, return expression is '%s', but function expects '%s'!",
    				ret->exp->inferred_type->name, ret_type_def->name);
            }
		}
	} else {
		if (fn->type->return_type != g_compiler->type_def_void)
			ERROR(ret, "Return statment has no expression, but function returns '%s'!",
				ret_type_def->name);
	}
}

void Type_Checking::check_type (Ast_Expression* exp) {
    switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION:
            check_type(static_cast<Ast_Type_Definition*>(exp));
            break;
        case AST_EXPRESSION_FUNCTION:
            check_type(static_cast<Ast_Function*>(exp));
            break;
        case AST_EXPRESSION_CALL:
            check_type(static_cast<Ast_Function_Call*>(exp));
            break;
        case AST_EXPRESSION_CAST:
            check_type(static_cast<Ast_Cast*>(exp));
            break;
        case AST_EXPRESSION_BINARY:
            check_type(static_cast<Ast_Binary*>(exp));
            break;
        case AST_EXPRESSION_UNARY:
            check_type(static_cast<Ast_Unary*>(exp));
            break;
        case AST_EXPRESSION_IDENT:
            check_type(static_cast<Ast_Ident*>(exp));
            break;
        case AST_EXPRESSION_LITERAL:
            check_type(static_cast<Ast_Literal*>(exp));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Cast* cast) {
	check_type(cast->value);
	check_type(cast->cast_to);
	if (cast->cast_to->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
		auto type_def = static_cast<Ast_Type_Definition*>(cast->cast_to);
		cast->inferred_type = type_def;
	} else {
		ERROR(cast, "Cast target is not a type");
	}
}

void Type_Checking::check_type (Ast_Type_Definition* tydef) {
    switch (tydef->typedef_type) {
		case AST_TYPEDEF_FUNCTION:
            check_type(static_cast<Ast_Function_Type*>(tydef));
            break;
        case AST_TYPEDEF_STRUCT:
            check_type(static_cast<Ast_Struct_Type*>(tydef));
            break;
        case AST_TYPEDEF_ARRAY:
            check_type(static_cast<Ast_Array_Type*>(tydef));
            break;
        case AST_TYPEDEF_POINTER:
            check_type(static_cast<Ast_Pointer_Type*>(tydef));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Function_Type* ty) {
    ty->inferred_type = g_compiler->type_def_type;

	check_type(ty->return_type);
	for (int i = 0; i < ty->parameter_decls.size(); i++) {
		check_type(ty->parameter_decls[i]);
	}
}

void compute_struct_size (Ast_Struct_Type* _struct) {
	if (_struct->byte_size == 0) {
		size_t byte_offset = 0;
		for (size_t i = 0; i < _struct->attributes.size(); i++) {
			auto decl = _struct->attributes[i];
			decl->attribute_byte_offset = byte_offset;
			decl->attribute_index = i;

			assert (decl->type->exp_type == AST_EXPRESSION_TYPE_DEFINITION);
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->type);
			byte_offset += defn_ty->byte_size;
		}
		_struct->byte_size = byte_offset;
	}
}

void Type_Checking::check_type (Ast_Struct_Type* _struct) {
    _struct->inferred_type = g_compiler->type_def_type;
	for (auto decl : _struct->attributes) {
		check_type(decl);
	}
    compute_struct_size(_struct);
}

void Type_Checking::check_type (Ast_Array_Type* arr) {
    arr->inferred_type = g_compiler->type_def_type;
	check_type(arr->base);
	if (arr->count->exp_type == AST_EXPRESSION_LITERAL) {
		auto lit = static_cast<Ast_Literal*>(arr->count);
		if (lit->literal_type == AST_LITERAL_UNSIGNED_INT) {
			auto type_def = static_cast<Ast_Type_Definition*>(arr->base);
			arr->byte_size = arr->length() * type_def->byte_size;
		} else {
			ERROR(arr, "Arrays size must be an unsigned integer!");
		}
	} else {
		ERROR(arr, "Arrays can only have constant size!");
	}
}

void Type_Checking::check_type (Ast_Pointer_Type* ty) {
    ty->inferred_type = g_compiler->type_def_type;
	check_type(ty->base);
	if (ty->base->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(ty->base);
		delete ty->base;
		ty->base = ident->declaration->expression;
	}
}

void Type_Checking::check_type (Ast_Function* func) {
    func->inferred_type = func->type;
	check_type(func->type);
	if (func->scope) check_type(func->scope);
}

void Type_Checking::check_type (Ast_Function_Call* call) {
    if (call->fn->exp_type != AST_EXPRESSION_FUNCTION)
		ERROR(call, "Function calls can only be performed to functions types");

	auto func = static_cast<Ast_Function*>(call->fn);
	auto ret_ty = static_cast<Ast_Type_Definition*>(func->type->return_type);
	call->inferred_type = ret_ty;

	if (call->arguments.size() == func->type->parameter_decls.size()) {
		for (int i = 0; i < call->arguments.size(); i++) {
			auto param_decl = func->type->parameter_decls[i];
			auto param_decl_type = static_cast<Ast_Type_Definition*>(param_decl->type);
			auto param_exp = call->arguments[i];
			check_type(param_exp);

			if (param_decl_type != param_exp->inferred_type) {
				if (!cast_if_possible(&call->arguments[i], param_exp->inferred_type, param_decl_type)) {
					ERROR(call, "Type mismatch on parameter #%d, expected '%s' but got '%s'",
						i, param_decl_type->name, param_exp->inferred_type->name);
				}
			}
		}
	} else {
		ERROR(call, "Wrong number of arguments, function has %d, but call has %d",
			func->type->parameter_decls.size(), call->arguments.size());
	}
}

void Type_Checking::check_type (Ast_Binary* binop) {
    check_type(binop->lhs);
    if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
        // Now that we (should) know the type of the LHS,
        // we can set all the values for the RHS
		auto type_def = binop->lhs->inferred_type;
		// If we're trying to access an attribute from a pointer,
		// we follow that pointer until we're on the actual type
		if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_POINTER) {
			uint8_t deref_count = 0;
			while (type_def->typedef_type == AST_TYPEDEF_POINTER) {
				auto ptr_type = static_cast<Ast_Pointer_Type*>(type_def);
				type_def = static_cast<Ast_Type_Definition*>(ptr_type->base);
				deref_count += 1;
			}
			if (deref_count > WARN_MAX_DEREF_COUNT) {
				report_warning(&binop->location, "Attribute access on deep pointer (%d)", deref_count);
			}
		}

		if (type_def->typedef_type == AST_TYPEDEF_STRUCT) {
            auto _struct = static_cast<Ast_Struct_Type*>(type_def);
            if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(binop->rhs);
                auto attribute = _struct->find_attribute(ident->name);
                if (attribute) {
                    auto attr_type = static_cast<Ast_Type_Definition*>(attribute->type);
                    ident->inferred_type = attr_type;
                    binop->inferred_type = attr_type;
                    ident->declaration = attribute;
                } else {
                    ERROR(binop, "The type '%s' has no attribute named '%s'",
                        _struct->name, ident->name);
                }
            } else {
                // TODO: move this check to somewhere more relevant
                ERROR(binop, "Right of attribute access is NOT an identifier!");
            }
        } else if (type_def->typedef_type == AST_TYPEDEF_ARRAY) {
			auto _array = static_cast<Ast_Array_Type*>(type_def);
			if (binop->rhs->exp_type == AST_EXPRESSION_IDENT) {
				auto ident = static_cast<Ast_Ident*>(binop->rhs);
				if (strcmp(ident->name, "length") == 0) {
					binop->inferred_type = g_compiler->type_def_u64;
				} else if (strcmp(ident->name, "data") == 0) {
					auto ptr_type = new Ast_Pointer_Type();
					ptr_type->inferred_type = g_compiler->type_def_type;
					ptr_type->base = _array->base;
					binop->inferred_type = g_compiler->types->get_unique_pointer_type(ptr_type);
					g_compiler->types->compute_type_name_if_needed(binop->inferred_type);
				} else {
					ERROR(binop->rhs, "'%s' is not a valid attribute for array (use length or data)",
						ident->name);
				}
			} else {
				// TODO: move this check to somewhere more relevant
				ERROR(binop, "Right of attribute access is NOT an identifier!");
			}
		} else {
            ERROR(binop, "Left of attribute access has invalid type: '%s'",
				type_def->name);
        }
    } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
		if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
			auto arr_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
			binop->inferred_type = static_cast<Ast_Type_Definition*>(arr_type->base);

			check_type(binop->rhs);
			if (!cast_if_possible(&binop->rhs, binop->rhs->inferred_type, g_compiler->type_def_u64)) {
				ERROR(binop, "Type '%s' cannot be casted to u64 (index)",
					binop->rhs->inferred_type->name);
			}
		} else if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_STRUCT) {
			auto _struct = static_cast<Ast_Struct_Type*>(binop->lhs->inferred_type);
			if (_struct->is_slice) {
				auto data_decl = _struct->find_attribute("data");
				if (data_decl) {
					check_type(binop->rhs);
					auto ptr_type = static_cast<Ast_Pointer_Type*>(data_decl->type);
					binop->inferred_type = static_cast<Ast_Type_Definition*>(ptr_type->base);
				} else {
					ERROR(binop, "Slice type doesn't have data attribute");
				}
			} else {
				ERROR(binop, "Left struct is not a slice");
			}
		} else {
			ERROR(binop, "Left of array access is not of array or slice type");
		}
	} else {
    	check_type(binop->rhs);
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
    		case AST_BINARY_GTE: {
    			binop->inferred_type = g_compiler->type_def_bool;
    			break;
    		}
    		default: {
    			binop->inferred_type = binop->lhs->inferred_type;
    			break;
    		}
    	}
    }
}

void Type_Checking::check_type (Ast_Unary* unop) {
	check_type(unop->exp);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: {
			auto inst = g_compiler;
			if (unop->exp->inferred_type == inst->type_def_u8) {
				unop->inferred_type = inst->type_def_s16;
			} else if (unop->exp->inferred_type == inst->type_def_u16) {
				unop->inferred_type = inst->type_def_s32;
			} else if (unop->exp->inferred_type == inst->type_def_u32) {
				unop->inferred_type = inst->type_def_s64;
			} else if (unop->exp->inferred_type == inst->type_def_u64) {
				unop->inferred_type = inst->type_def_s64;
			} else {
				unop->inferred_type = unop->exp->inferred_type;
			}
            break;
		}
		case AST_UNARY_DEREFERENCE: {
            auto inf_type = unop->exp->inferred_type;
            if (inf_type->typedef_type == AST_TYPEDEF_POINTER) {
                auto ptr_type = static_cast<Ast_Pointer_Type*>(inf_type);
                auto base_type = static_cast<Ast_Type_Definition*>(ptr_type->base);
                unop->inferred_type = base_type;
            } else {
                ERROR(unop, "Can't dereference a non-pointer type expression!");
            }
            break;
		}
		case AST_UNARY_REFERENCE: {
		    auto ptr_type = new Ast_Pointer_Type();
		    ptr_type->inferred_type = g_compiler->type_def_type;
		    ptr_type->base = unop->exp->inferred_type;
		    unop->inferred_type = g_compiler->types->get_unique_pointer_type(ptr_type);
			g_compiler->types->compute_type_name_if_needed(unop->inferred_type);
            break;
		}
	}
}

void Type_Checking::check_type (Ast_Ident* ident) {
	if (ident->declaration) {
		ident->inferred_type = static_cast<Ast_Type_Definition*>(ident->declaration->type);
	} else {
		ERROR(ident, "Indetifier '%s' has no declaration!", ident->name);
	}
}

void Type_Checking::check_type (Ast_Literal* lit) {
    if (!lit->inferred_type) {
		auto inst = g_compiler;
        switch (lit->literal_type) {
            case AST_LITERAL_UNSIGNED_INT: {
				if (lit->uint_value <= UINT32_MAX) {
					if (lit->uint_value <= UINT16_MAX) {
						if (lit->uint_value <= UINT8_MAX) {
							lit->inferred_type = inst->type_def_u8;
						} else lit->inferred_type = inst->type_def_u16;
					} else lit->inferred_type = inst->type_def_u32;
				} else lit->inferred_type = inst->type_def_u64;
                break;
            }
            case AST_LITERAL_SIGNED_INT: {
				if (lit->int_value <= INT32_MAX && lit->int_value >= INT32_MIN) {
					if (lit->int_value <= INT16_MAX && lit->int_value >= INT16_MIN) {
						if (lit->int_value <= INT8_MAX && lit->int_value >= INT8_MIN) {
							lit->inferred_type = inst->type_def_s8;
						} else lit->inferred_type = inst->type_def_s16;
					} else lit->inferred_type = inst->type_def_s32;
				} else lit->inferred_type = inst->type_def_s64;
                break;
            }
            case AST_LITERAL_DECIMAL: {
				lit->inferred_type = g_compiler->type_def_f32;
                break;
            }
            case AST_LITERAL_STRING: {
				// TODO: Pre-allocate this pointer, since it's very common
				lit->inferred_type = new Ast_Pointer_Type(g_compiler->type_def_u8);
                break;
            }
            default: break;
        }
    }
}
