#pragma once

#include "ast/nodes.hpp"

struct Ast_Utils {
    static Ast_Declaration* find_declaration (Ast_Scope* scope, const char* name, bool use_imports, bool recurse) {
        Ast_Declaration* decl = Ast_Utils::find_local_declaration(scope, name);
        if (decl) return decl;

        // @INFO if not found in current scope, we check if we belong to a function
        // declaration, if so check if we can resolve using argument declarations
        if (scope->scope_of) {
            decl = Ast_Utils::find_local_declaration(scope->scope_of->arg_scope, name);

            // @INFO if no matching argument declaration is found, recurse back to the
            // global scope but only for constants, since we're outside the function
            if (!decl) {
                decl = Ast_Utils::find_const_declaration(scope->parent, name);
                if (decl) return decl;
            }

            // @INFO if no constant match is found in any of the parent scopes, we
            // look for variables in the global scope, we can always see globals
            if (!decl) {
                auto global_scope = Ast_Utils::get_global_scope(scope);
                return Ast_Utils::find_declaration(global_scope, name, true, true);
            } else return decl;
        }

        // If import use is requested we look for declarations in the global scope
        // of every imported scope, we ignore import's parents & imports
        if (use_imports) {
            For (scope->imports) {
                decl = Ast_Utils::find_local_declaration(it, name);
                if (decl) return decl;
            }
        }

        // @INFO if we couldn't find any matching declaration until here, we
        // have to recurse to our parent scope, and do the same again
        if (recurse && scope->parent) {
            return Ast_Utils::find_declaration(scope->parent, name, use_imports, recurse);
        } else return NULL;
    }

    static Ast_Declaration* find_var_declaration (Ast_Scope* scope, const char* name) {
        For3 (scope->statements, stm, j) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                if (!decl->is_constant) {
                    for (size_t i = 0; i < decl->names.size; i++) {
                        auto decl_name = decl->names[i];
                        if (decl_name && strcmp(decl_name, name) == 0) {
                            return decl;
                        }
                    }
                }
            }
        }
        
        if (scope->scope_of) {
            auto decl = Ast_Utils::find_local_declaration(scope->scope_of->arg_scope, name);
            if (decl) return decl;

            auto global_scope = Ast_Utils::get_global_scope(scope);
            return Ast_Utils::find_var_declaration(global_scope, name);
        }

        if (scope->parent) {
            return Ast_Utils::find_var_declaration(scope->parent, name);
        } else return NULL;
    }

    static Ast_Declaration* find_const_declaration (Ast_Scope* scope, const char* name) {
        For3 (scope->statements, stm, j) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                if (decl->is_constant) {
                    for (size_t i = 0; i < decl->names.size; i++) {
                        auto decl_name = decl->names[i];
                        if (decl_name && strcmp(decl_name, name) == 0) {
                            return decl;
                        }
                    }
                }
            }
        }

        if (scope->parent) {
            return Ast_Utils::find_const_declaration(scope->parent, name);
        } else return NULL;
    }

    static Ast_Declaration* find_local_declaration (Ast_Scope* scope, const char* name) {
        For3 (scope->statements, stm, j) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                for (size_t i = 0; i < decl->names.size; i++) {
                    auto decl_name = decl->names[i];

                    if (decl_name && strcmp(decl_name, name) == 0) {
                        return decl;
                    }
                }
            }
        }
        return NULL;
    }

    static void get_all_declarations (Ast_Scope* scope, String_Map<Array<Ast_Declaration*>>* decl_map) {
        For3 (scope->statements, stm, j) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                for (size_t i = 0; i < decl->names.size; i++) {
                    auto decl_name = decl->names[i];
                    if (decl_name) {
                        (*decl_map)[decl_name].push(decl);
                    }
                }
            }
        }
    }

    static bool has_static_ifs (Ast_Scope* scope) {
        For2 (scope->statements, stm) {
            if (stm->stm_type == AST_STATEMENT_STATIC_IF) {
                return true;
            }
        }
        return false;
    }

    static bool any_import_has_static_ifs (Ast_Scope* scope) {
        For (scope->imports) {
            if (Ast_Utils::has_static_ifs(it)) {
                return true;
            }
        }
		return false;
    }

	static bool is_ancestor_of (Ast_Scope* ancestor, Ast_Scope* child) {
		if (ancestor == child) return true;

		while (child->parent) {
			child = child->parent;
			if (ancestor == child) {
				return true;
			}
		}
		return false;
	}

	static Ast_Function* get_parent_function (Ast_Scope* scope) {
        if (scope->scope_of) return scope->scope_of;

		while (scope->parent) {
			scope = scope->parent;
			if (scope->scope_of) {
				return scope->scope_of;
			}
		}
		return NULL;
	}

	static Ast_Scope* get_global_scope (Ast_Scope* scope) {
        while (scope->parent != NULL) {
            scope = scope->parent;
        }
		return scope;
	}

    static Ast_Declaration* find_attribute (Ast_Struct_Type* struct_type, const char* name) {
        return Ast_Utils::find_local_declaration(&struct_type->scope, name);
    }
};