#include "ast/ast.hpp"

#include "ast/types.hpp"

uint64_t Ast::node_count = 0;

bool Ast_Arguments::add (Ast_Expression* exp) {
    if (exp->exp_type == AST_EXPRESSION_BINARY) {
        auto binary = static_cast<Ast_Binary*>(exp);
        if (binary->binary_op == AST_BINARY_ASSIGN) {
            if (binary->lhs->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(binary->lhs);
                this->named[ident->name] = binary->rhs;
                return true;
            }
        }
    }
    this->unnamed.push_back(exp);
    return false;
}

Ast_Declaration* Ast_Scope::find_declaration (const char* _name, bool use_includes, bool use_imports, bool recurse) {
    Ast_Declaration* decl = NULL;

    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            decl = static_cast<Ast_Declaration*>(stm);
            if (strcmp(decl->name, _name) == 0) {
                return decl;
            }
        }
    }
    if (this->scope_of) {
		for (auto arg_decl : this->scope_of->type->arg_decls) {
			if (strcmp(arg_decl->name, _name) == 0) {
				return arg_decl;
			}
		}
        decl = this->parent->find_const_declaration(_name);
        if (!decl) {
            return this->get_global_scope()->find_declaration(_name, true, true, false);
        } else return decl;
	}

    if (use_includes) {
        for (auto scope : this->includes) {
            decl = scope->find_declaration(_name, true, false, false);
            if (decl) return decl;
        }
    }

    if (use_imports) {
        for (auto scope : this->imports) {
            decl = scope->find_declaration(_name, true, false, false);
            if (decl) return decl;
        }
    }

    if (recurse && this->parent) {
        return this->parent->find_declaration(_name, use_includes, use_imports, recurse);
    } else return NULL;
}

Ast_Declaration* Ast_Scope::find_var_declaration (const char* _name) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (!decl->is_constant && strcmp(decl->name, _name) == 0) {
                return decl;
            }
        }
    }
    if (this->scope_of) {
		for (auto arg_decl : this->scope_of->type->arg_decls) {
			if (strcmp(arg_decl->name, _name) == 0) {
				return arg_decl;
			}
		}
        return this->get_global_scope()->find_var_declaration(_name);
	}

    if (this->parent) {
        return this->parent->find_var_declaration(_name);
    } else return NULL;
}

Ast_Declaration* Ast_Scope::find_const_declaration (const char* _name) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->is_constant && strcmp(decl->name, _name) == 0) {
                return decl;
            }
        }
    }

    if (this->parent) {
        return this->parent->find_const_declaration(_name);
    } else return NULL;
}

Ast_Function* Ast_Scope::get_parent_function () {
	if (this->scope_of) return this->scope_of;
	else {
		Ast_Scope* block = this;
		while (block->parent) {
			block = block->parent;
			if (block->scope_of) return block->scope_of;
		}
		return NULL;
	}
}

void Ast_Foreign::add (Ast_Statement* stm) {
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);
        this->add(decl);
    } else printf("Only declarations can go inside #foreign scopes");
}

void Ast_Foreign::add (Ast_Declaration* decl) {
    if (!decl->is_constant) printf("Declarations inside #foreign scope must be function types");
    if (!decl->expression) printf("Declarations inside #foreign scope must have values");
    if (decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
        auto type = static_cast<Ast_Type_Instance*>(decl->expression);
        if (type->typedef_type == AST_TYPEDEF_FUNCTION) {
            auto func = new Ast_Function();
            func->location = type->location;
            func->type = static_cast<Ast_Function_Type*>(type);
            func->foreign_function_name = this->get_foreign_function_name(decl->name);
            func->foreign_module_name = this->module_name;
            func->name = decl->name;

            decl->expression = func;

            this->declarations.push_back(decl);
        } else printf("Only function types can go inside #foreign scope declarations");
    } else printf("Only types can go inside #foreign scope declarations");
}

Ast_Declaration* Ast_Struct_Type::find_attribute (const char* _name) {
	for (auto decl : this->attributes) {
		if (strcmp(decl->name, _name) == 0) return decl;
	}
	return NULL;
}

// TODO: precompute depth for each pointer type (when uniqued?)
Ast_Type_Instance* Ast_Pointer_Type::get_base_type_recursive() {
    auto base_type = static_cast<Ast_Type_Instance*>(this->base);

    uint8_t deref_count = 0;
    while (base_type->typedef_type == AST_TYPEDEF_POINTER) {
        auto ptr_type = static_cast<Ast_Pointer_Type*>(base_type);
        base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
        deref_count += 1;
    }

    if (deref_count > WARN_MAX_DEREF_COUNT) {
        printf("Attribute access on deep pointer (%d)", deref_count);
    }

    return base_type;
}

Ast_Slice_Type::Ast_Slice_Type(Ast_Expression* base_type, const char* name) {
	this->typedef_type = AST_TYPEDEF_STRUCT;
	this->is_slice = true;
    this->name = name;

    auto ptr_type = new Ast_Pointer_Type(base_type);
    auto length_decl = ast_make_declaration_with_type("length", Types::type_u64);
    auto data_decl = ast_make_declaration_with_type("data", ptr_type);

    this->attributes.push_back(length_decl);
    this->attributes.push_back(data_decl);
}

Ast_Binary_Type token_to_binop (Token_Type tType) {
	switch (tType) {
		case TOKEN_EQUAL: 			return AST_BINARY_ASSIGN;
		case TOKEN_DOT: 			return AST_BINARY_ATTRIBUTE;
		case TOKEN_SQ_BRAC_OPEN: 	return AST_BINARY_SUBSCRIPT;

		case TOKEN_DOUBLE_AMP:		return AST_BINARY_LOGICAL_AND;
		case TOKEN_DOUBLE_PIPE:		return AST_BINARY_LOGICAL_OR;

		case TOKEN_ADD: 			return AST_BINARY_ADD;
		case TOKEN_SUB: 			return AST_BINARY_SUB;
		case TOKEN_MUL: 			return AST_BINARY_MUL;
		case TOKEN_DIV: 			return AST_BINARY_DIV;
		case TOKEN_PERCENT:			return AST_BINARY_REM;

		case TOKEN_AMP:				return AST_BINARY_BITWISE_AND;
		case TOKEN_PIPE:			return AST_BINARY_BITWISE_OR;
		case TOKEN_CARET:			return AST_BINARY_BITWISE_XOR;
		case TOKEN_RIGHT_SHIFT:		return AST_BINARY_BITWISE_RIGHT_SHIFT;
		case TOKEN_LEFT_SHIFT:		return AST_BINARY_BITWISE_LEFT_SHIFT;

		case TOKEN_DOUBLE_EQUAL:	return AST_BINARY_EQ;
		case TOKEN_NOT_EQUAL:		return AST_BINARY_NEQ;
		case TOKEN_GREATER_EQUAL:	return AST_BINARY_GTE;
		case TOKEN_LESSER_EQUAL:	return AST_BINARY_LTE;
		case TOKEN_GREATER:			return AST_BINARY_GT;
		case TOKEN_LESSER:			return AST_BINARY_LT;
		default: 					return AST_BINARY_UNINITIALIZED;
	};
}

Ast_Type_Instance* Ast_Binary::get_result_type() {
    switch (this->binary_op) {
        case AST_BINARY_EQ:
        case AST_BINARY_NEQ:
        case AST_BINARY_LT:
        case AST_BINARY_LTE:
        case AST_BINARY_GT:
        case AST_BINARY_GTE: 	return Types::type_bool;
        default: 				return this->lhs->inferred_type;
    }
}

short Ast_Binary::get_precedence (Token_Type opToken) {
	switch (opToken) {
		default: 		  			return 0;
		case TOKEN_EQUAL: 			return 1;
		case TOKEN_SQ_BRAC_OPEN:	return 2;
		case TOKEN_DOUBLE_PIPE:		return 3;
		case TOKEN_DOUBLE_AMP:		return 4;
		case TOKEN_PIPE:			return 5;
		case TOKEN_TILDE:			return 6;
		case TOKEN_AMP:				return 7;

		case TOKEN_DOUBLE_EQUAL:
		case TOKEN_NOT_EQUAL:		return 8;

		case TOKEN_GREATER_EQUAL:
		case TOKEN_LESSER_EQUAL:
		case TOKEN_GREATER:
		case TOKEN_LESSER:			return 9;

		case TOKEN_RIGHT_SHIFT:
		case TOKEN_LEFT_SHIFT:		return 10;

		case TOKEN_ADD:
		case TOKEN_SUB:   			return 11;

		case TOKEN_MUL:
		case TOKEN_DIV:
		case TOKEN_PERCENT:			return 12;

		case TOKEN_CARET:
		case TOKEN_EXCLAMATION:		return 13;

		case TOKEN_DOUBLE_ADD:
		case TOKEN_DOUBLE_SUB:
		case TOKEN_DOT:   			return 14;

	}
}

bool Ast_Binary::is_left_associative (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: 			return true;
		default: 		  			return false;
	}
}

const char* _get_type_name (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(exp);
		return ident->name;
	} else if (exp->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
		auto type_inst = static_cast<Ast_Type_Instance*>(exp);
		ast_compute_type_name_if_needed(type_inst);
		return type_inst->name;
	} else abort();
}

void ast_compute_type_name_if_needed (Ast_Type_Instance* type_inst) {
	if (!type_inst->name) {
		switch (type_inst->typedef_type) {
	        case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type_inst);
				if (_struct->is_slice) {
					auto slice = static_cast<Ast_Slice_Type*>(type_inst);
					auto base_name = _get_type_name(slice->get_base());
	        		auto base_name_length = strlen(base_name);
                    auto tmp = (char*) malloc(base_name_length + 3);
                    sprintf_s(tmp, base_name_length + 23, "[]%s", base_name);
                    slice->name = tmp;
				}
				break;
			}
	        case AST_TYPEDEF_POINTER: {
	            auto _ptr = static_cast<Ast_Pointer_Type*>(type_inst);
	            if (_ptr->name == NULL) {
					auto base_name = _get_type_name(_ptr->base);
	        		auto base_name_length = strlen(base_name);
	        		auto tmp = (char*) malloc(base_name_length + 2);
                    sprintf_s(tmp, base_name_length + 23, "*%s", base_name);
                    _ptr->name = tmp;
	        	}
				break;
	        }
	        case AST_TYPEDEF_ARRAY: {
	            auto _arr = static_cast<Ast_Array_Type*>(type_inst);
	            if (_arr->name == NULL) {
					auto base_name = _get_type_name(_arr->base);
	        		auto base_name_length = strlen(base_name);
					auto tmp = (char*) malloc(base_name_length + 23);
					sprintf_s(tmp, base_name_length + 23, "[%lld]%s", _arr->length_uint, base_name);
                    _arr->name = tmp;
	        	}
				break;
	        }
	        case AST_TYPEDEF_FUNCTION: {
	            auto _func = static_cast<Ast_Function_Type*>(type_inst);
	            if (_func->name == NULL) {
	        		auto arg_decls = _func->arg_decls;

	        		size_t name_size = strlen("fn (");
	        		if (arg_decls.size() > 0) {
						auto param_name = _get_type_name(arg_decls[0]->type);
	        			name_size += strlen(param_name);
	        			for (int i = 1; i < arg_decls.size(); i++) {
	        				name_size += strlen(", ");
	        				param_name = _get_type_name(arg_decls[i]->type);
	        				name_size += strlen(param_name);
	        			}
	        		}
	        		name_size += strlen(") -> ");
					auto ret_name = _get_type_name(_func->ret_type);
	        		name_size += strlen(ret_name);
	        		auto tmp = (char*) malloc(name_size + 1);

	        		size_t offset = 0;
	        		memcpy(tmp, "fn (", 4);
	        		offset += 4;

					size_t par_type_name_length;
	        		if (arg_decls.size() > 0) {
						auto param_name = _get_type_name(arg_decls[0]->type);
						par_type_name_length = strlen(param_name);
	        			memcpy(tmp + offset, param_name, par_type_name_length);
	        			offset += par_type_name_length;
	        			for (int i = 1; i < arg_decls.size(); i++) {
	        				memcpy(tmp + offset, ", ", 2);
	        				offset += 2;
	        				param_name = _get_type_name(arg_decls[i]->type);
							par_type_name_length = strlen(param_name);
	        				memcpy(tmp + offset, param_name, par_type_name_length);
	        				offset += par_type_name_length;
	        			}
	        		}

	        		memcpy(tmp + offset, ") -> ", 5);
	        		offset += 5;
					par_type_name_length = strlen(ret_name);
	        		memcpy(tmp + offset, ret_name, par_type_name_length);
	        		offset += par_type_name_length;
	        		tmp[offset] = '\0';

                    _func->name = tmp;
	        	}
	        }
	        default: break;
	    }
	}
}

Ast_Type_Instance* ast_get_container_signed (Ast_Type_Instance* unsigned_type) {
    if (unsigned_type == Types::type_u8) {
        return Types::type_s16;
    } else if (unsigned_type == Types::type_u16) {
        return Types::type_s32;
    } else if (unsigned_type == Types::type_u32) {
        return Types::type_s64;
    } else if (unsigned_type == Types::type_u64) {
        return Types::type_s64;
    } else return unsigned_type;
}

Ast_Struct_Type* ast_get_smallest_type (uint64_t value) {
    if (value <= UINT32_MAX) {
        if (value <= UINT16_MAX) {
            if (value <= UINT8_MAX) {
                return Types::type_u8;
            } else return Types::type_u16;
        } else return Types::type_u32;
    } else return Types::type_u64;
}

Ast_Struct_Type* ast_get_smallest_type (int64_t value) {
    if (value <= INT32_MAX && value >= INT32_MIN) {
        if (value <= INT16_MAX && value >= INT16_MIN) {
            if (value <= INT8_MAX && value >= INT8_MIN) {
                return Types::type_s8;
            } else return Types::type_s16;
        } else return Types::type_s32;
    } else return Types::type_s64;
}

Ast_Literal* ast_make_literal (const char* value) {
	auto lit = new Ast_Literal();
	lit->literal_type = AST_LITERAL_STRING;
	lit->string_value = value;
	return lit;
}

Ast_Literal* ast_make_literal (unsigned long long value) {
	auto lit = new Ast_Literal();
	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
	lit->uint_value = value;
	return lit;
}

Ast_Literal* ast_make_literal (bool value) {
	auto lit = new Ast_Literal();
    lit->inferred_type = Types::type_bool;
	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
	lit->uint_value = value;
	return lit;
}

Ast_Ident* ast_make_ident (const char* name) {
	auto out = new Ast_Ident();
	out->name = name;
	return out;
}

Ast_Unary* ast_make_unary (Ast_Unary_Type type, Ast_Expression* expression) {
    return new Ast_Unary(type, expression);
}

Ast_Binary* ast_make_binary (Ast_Binary_Type type, Ast_Expression* lhs, Ast_Expression* rhs) {
    auto binop = new Ast_Binary(type);
    binop->lhs = lhs;
    binop->rhs = rhs;
    return binop;
}

Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const) {
    auto decl = new Ast_Declaration();
    decl->is_constant = is_const;
    decl->type = exp->inferred_type;
    decl->expression = exp;
    decl->name = name;
    return decl;
}

Ast_Declaration* ast_make_declaration_with_type (const char* name, Ast_Expression* type) {
    auto decl = new Ast_Declaration();
    decl->type = type;
    decl->name = name;
    return decl;
}
