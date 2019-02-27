#include "ast/nodes.hpp"

Ast_Declaration* Ast_Scope::find_declaration (const char* _name, bool use_imports, bool recurse) {
    Ast_Declaration* decl = NULL;

    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            decl = static_cast<Ast_Declaration*>(stm);
            for (size_t i = 0; i < decl->names.size; i++) {
                auto decl_name = decl->names[i];
                if (decl_name && strcmp(decl_name, _name) == 0) {
                    return decl;
                }
            }

        }
    }
    if (this->scope_of) {
        decl = this->scope_of->arg_scope->find_declaration(_name, false, false);
        if (!decl) decl = this->parent->find_const_declaration(_name);
        if (!decl) {
            return this->get_global_scope()->find_declaration(_name, true, false);
        } else return decl;
	}

    if (use_imports) {
        for (auto scope : this->imports) {
            decl = scope->find_declaration(_name, false, false);
            if (decl) return decl;
        }
    }

    if (recurse && this->parent) {
        return this->parent->find_declaration(_name, use_imports, recurse);
    } else return NULL;
}

Ast_Declaration* Ast_Scope::find_var_declaration (const char* _name) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (!decl->is_constant) {
                for (size_t i = 0; i < decl->names.size; i++) {
                    auto decl_name = decl->names[i];
                    if (decl_name && strcmp(decl_name, _name) == 0) {
                        return decl;
                    }
                }
            }
        }
    }
    if (this->scope_of) {
		auto decl = this->scope_of->arg_scope->find_declaration(_name, false, false);
        if (decl) return decl;
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
            if (decl->is_constant) {
                for (size_t i = 0; i < decl->names.size; i++) {
                    auto decl_name = decl->names[i];
                    if (decl_name && strcmp(decl_name, _name) == 0) {
                        return decl;
                    }
                }
            }
        }
    }

    if (this->parent) {
        return this->parent->find_const_declaration(_name);
    } else return NULL;
}

void Ast_Scope::find_all_declarations (String_Map<std::vector<Ast_Declaration*>>* decl_map) {
    for (auto stm : this->statements) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            for (size_t i = 0; i < decl->names.size; i++) {
                auto decl_name = decl->names[i];
                if (decl_name) {
                    (*decl_map)[decl_name].push_back(decl);
                }
            }
        }
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

// TODO: precompute depth for each pointer type (when uniqued?)
Ast_Type* Ast_Pointer_Type::get_base_type_recursive() {
    auto base_type = static_cast<Ast_Type*>(this->base);

    uint8_t deref_count = 0;
    while (base_type->typedef_type == AST_TYPEDEF_POINTER) {
        auto ptr_type = static_cast<Ast_Pointer_Type*>(base_type);
        base_type = static_cast<Ast_Type*>(ptr_type->base);
        deref_count += 1;
    }

    return base_type;
}

Ast_Binary_Type token_to_binop (Token_Type tType) {
	switch (tType) {
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
