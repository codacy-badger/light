#include "ast/ast.hpp"

#include "compiler.hpp"

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

Ast_Expression* Ast_Arguments::get_named_value (const char* param_name) {
    for (auto entry : this->named) {
        if (strcmp(entry.first, param_name) == 0) {
            return entry.second;
        }
    }
    return NULL;
}

Ast_Expression* Ast_Arguments::get_unnamed_value (const size_t index) {
    if (index < this->unnamed.size()) {
        return this->unnamed[index];
    } else return NULL;
}

bool Ast_Statement::remove_note (const char* name) {
    auto it = this->notes.begin();
    while (it != this->notes.end()) {
        if (strcmp((*it), name) == 0) {
            this->notes.erase(it);
            return true;
        } else it++;
    }
    return NULL;
}

Ast_Declaration* Ast_Scope::find_declaration (const char* _name, bool is_const) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->is_constant == is_const && strcmp(decl->name, _name) == 0) {
				return decl;
			}
        }
    }

	if (!is_const && this->scope_of) {
		for (auto decl : this->scope_of->type->arg_decls) {
			if (strcmp(decl->name, _name) == 0) {
				return decl;
			}
		}
        // @TODO do we really need this? maybe changing the structure of
        // the scope struct we can avoid having this, since it conflicts
        // with the internal scope (u8, u16, u32, etc.)
        return this->find_global_declaration(_name, is_const);
	}

    auto decl = this->find_external_declaration(_name, is_const);
    if (decl) return decl;

    if (this->parent) {
        return this->parent->find_declaration(_name, is_const);
    } else return NULL;
}

Ast_Declaration* Ast_Scope::find_external_declaration (const char* _name, bool is_const) {
    for (auto scope : this->import_scopes) {
        auto decl = scope->find_non_const_declaration(_name);
        if (decl && decl->is_constant == is_const) return decl;
    }
    return NULL;
}

Ast_Declaration* Ast_Scope::find_non_const_declaration (const char* _name) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (!decl->is_constant && strcmp(decl->name, _name) == 0) {
				return decl;
			}
        }
    }
	if (this->scope_of && this->scope_of->type) {
		for (auto decl : this->scope_of->type->arg_decls) {
			if (strcmp(decl->name, _name) == 0) {
				return decl;
			}
		}
        // @TODO do we really need this? maybe changing the structure of
        // the scope struct we can avoid having this, since it conflicts
        // with the internal scope (u8, u16, u32, etc.)
        auto global_scope = this->get_global_scope();
        if (this != global_scope) {
            return global_scope->find_non_const_declaration(_name);
        }
	}
    for (auto scope : this->import_scopes) {
        auto decl = scope->find_non_const_declaration(_name);
        if (decl) return decl;
    }
    if (this->parent) {
        return this->parent->find_non_const_declaration(_name);
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
    for (auto scope : this->import_scopes) {
        auto decl = scope->find_const_declaration(_name);
        if (decl) return decl;
    }
    if (this->parent) {
        return this->parent->find_const_declaration(_name);
    } else return NULL;
}

Ast_Declaration* Ast_Scope::find_declaration (const char* name) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (strcmp(decl->name, name) == 0) {
				return decl;
			}
        }
    }
    return NULL;
}

bool Ast_Scope::is_ancestor (Ast_Scope* other) {
	if (this == other) return true;
	else {
		Ast_Scope* block = this;
		while (block->parent) {
			block = block->parent;
			if (block == other) return true;
		}
		return false;
	}
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

void Ast_Directive_Foreign::add (Ast_Statement* stm) {
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);
        this->add(decl);
    } else ERROR_STOP(stm, "Only declarations can go inside #foreign scopes");
}

void Ast_Directive_Foreign::add (Ast_Declaration* decl) {
    if (!decl->is_constant) ERROR_STOP(decl, "Declarations inside #foreign scope must be function types");
    if (!decl->expression) ERROR_STOP(decl, "Declarations inside #foreign scope must have values");
    if (decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
        auto type = static_cast<Ast_Type_Instance*>(decl->expression);
        if (type->typedef_type == AST_TYPEDEF_FUNCTION) {
            auto func = new Ast_Function();
            func->type = static_cast<Ast_Function_Type*>(type);
            func->foreign_function_name = this->get_foreign_function_name(decl->name);
            func->foreign_module_name = this->module_name;
            func->name = decl->name;

            decl->expression = func;

            this->declarations.push_back(decl);
        } else ERROR_STOP(decl, "Only function types can go inside #foreign scope declarations");
    } else ERROR_STOP(decl, "Only types can go inside #foreign scope declarations");
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
        WARN(this, "Attribute access on deep pointer (%d)", deref_count);
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

Ast_Struct_Type* build_type_any () {
    auto type_any = new Ast_Struct_Type("any");

    auto type_decl = ast_make_declaration_with_type("type", Types::type_u8);
    type_any->attributes.push_back(type_decl);

    auto ptr_type = new Ast_Pointer_Type(Types::type_void);
    auto value_decl = ast_make_declaration_with_type("value", ptr_type);
    type_any->attributes.push_back(value_decl);

    return type_any;
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
					sprintf_s(tmp, base_name_length + 23, "[%lld]%s", _arr->get_length(), base_name);
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

bool ast_function_types_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
    if (func_type1->arg_decls.size() != func_type2->arg_decls.size()) return false;

    for (size_t i = 0; i < func_type1->arg_decls.size(); i++) {
        auto arg_type1 = static_cast<Ast_Type_Instance*>(func_type1->arg_decls[i]->type);
        auto arg_type2 = static_cast<Ast_Type_Instance*>(func_type2->arg_decls[i]->type);
        if (!ast_types_are_equal(arg_type1, arg_type2)) return false;
    }

    auto ret_type1 = static_cast<Ast_Type_Instance*>(func_type1->ret_type);
    auto ret_type2 = static_cast<Ast_Type_Instance*>(func_type2->ret_type);
    return ast_types_are_equal(ret_type1, ret_type2);
}

// TODO: solve this: either we unique all types or we make deep comparisons
bool ast_types_are_equal (Ast_Type_Instance* type_inst1, Ast_Type_Instance* type_inst2) {
    //return type_inst1 == type_inst2;
    if (type_inst1 == type_inst2) return true;
    else {
        if (type_inst1->typedef_type != type_inst2->typedef_type) return false;
        switch (type_inst1->typedef_type) {
            case AST_TYPEDEF_STRUCT: return false;
            case AST_TYPEDEF_POINTER: {
                auto ptr_type1 = static_cast<Ast_Pointer_Type*>(type_inst1);
                auto ptr_type2 = static_cast<Ast_Pointer_Type*>(type_inst2);
                auto base_type1 = static_cast<Ast_Type_Instance*>(ptr_type1->base);
                auto base_type2 = static_cast<Ast_Type_Instance*>(ptr_type2->base);
                return ast_types_are_equal(base_type1, base_type2);
            }
            case AST_TYPEDEF_ARRAY: {
                auto arr_type1 = static_cast<Ast_Array_Type*>(type_inst1);
                auto arr_type2 = static_cast<Ast_Array_Type*>(type_inst2);

                if (arr_type1->get_length() != arr_type2->get_length()) return false;

                auto base_type1 = static_cast<Ast_Type_Instance*>(arr_type1->base);
                auto base_type2 = static_cast<Ast_Type_Instance*>(arr_type2->base);
                return ast_types_are_equal(base_type1, base_type2);
            }
            case AST_TYPEDEF_FUNCTION: {
                auto func_type1 = static_cast<Ast_Function_Type*>(type_inst1);
                auto func_type2 = static_cast<Ast_Function_Type*>(type_inst2);
                return ast_function_types_are_equal(func_type1, func_type2);
            }
            default: abort();
        }
    }
}

bool try_cast (Ast_Expression** exp_ptr, Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
	if (ast_types_are_equal(type_from, type_to)) return true;
	else if (Compiler::inst->types->is_implicid_cast(type_from, type_to)) {
        auto cast = new Ast_Cast();
		cast->location = (*exp_ptr)->location;
        cast->value = (*exp_ptr);
        cast->cast_to = type_to;
        cast->inferred_type = type_to;

		// INFO: if the cast comes from an implicid array cast, mark it!
		if (type_from->typedef_type == AST_TYPEDEF_ARRAY) {
			cast->is_array_to_slice_cast = true;
		}
        // INFO: if the cast comes from an implicid any cast, mark it!
        if (type_to == Types::type_any) {
            cast->is_value_to_any_cast = true;
        }

        (*exp_ptr) = cast;
        return true;
    } else return false;
}

bool try_cast (Ast_Expression** exp_ptr, Ast_Type_Instance* type_to) {
    return try_cast(exp_ptr, (*exp_ptr)->inferred_type, type_to);
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
