#pragma once

#include "ast/nodes.hpp"

struct Ast_Utils {
    static Ast_Declaration* find_declaration (Ast_Scope* scope, const char* _name, bool use_imports, bool recurse) {
        Ast_Declaration* decl = NULL;

        // @INFO loop inside current scope to find the requested declaration
        for (auto stm : scope->statements) {
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

        // @INFO if not found in current scope, we check if we belong to a function
        // declaration, if so check if we can resolve using argument declarations
        if (scope->scope_of) {
            decl = Ast_Utils::find_declaration(scope->scope_of->arg_scope, _name, false, false);

            // @INFO if no matching argument declaration is found, recurse back to the
            // global scope but only for constants, since we're outside the function
            if (!decl) {
                decl = Ast_Utils::find_const_declaration(scope->parent, _name);
                if (decl) return decl;
            }

            // @INFO if no constant match is found in any of the parent scopes, we
            // look for variables in the global scope, we can always see globals
            if (!decl) {
                return Ast_Utils::find_declaration(scope->get_global_scope(), _name, true, false);
            } else return decl;
        }

        // If import use is requested we look for declarations in the global scope
        // of every imported scope, we ignore import's parents & imports
        if (use_imports) {
            for (auto imported_scope : scope->imports) {
                decl = Ast_Utils::find_declaration(imported_scope, _name, false, false);
                if (decl) return decl;
            }
        }

        // @INFO if we couldn't find any matching declaration until here, we
        // have to recurse to our parent scope, and do the same again
        if (recurse && scope->parent) {
            return Ast_Utils::find_declaration(scope->parent, _name, use_imports, recurse);
        } else return NULL;
    }

    static Ast_Declaration* find_var_declaration (Ast_Scope* scope, const char* _name) {
        for (auto stm : scope->statements) {
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
        
        if (scope->scope_of) {
            auto decl = Ast_Utils::find_declaration(scope->scope_of->arg_scope, _name, false, false);
            if (decl) return decl;
            return Ast_Utils::find_var_declaration(scope->get_global_scope(), _name);
        }

        if (scope->parent) {
            return Ast_Utils::find_var_declaration(scope->parent, _name);
        } else return NULL;
    }

    static Ast_Declaration* find_const_declaration (Ast_Scope* scope, const char* _name) {
        for (auto stm : scope->statements) {
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

        if (scope->parent) {
            return Ast_Utils::find_const_declaration(scope->parent, _name);
        } else return NULL;
    }

    static Ast_Declaration* find_local_declaration (Ast_Scope* scope, const char* _name) {
        for (auto stm : scope->statements) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                for (size_t i = 0; i < decl->names.size; i++) {
                    auto decl_name = decl->names[i];

                    if (decl_name && strcmp(decl_name, _name) == 0) {
                        return decl;
                    }
                }
            }
        }
        return NULL;
    }

    static Ast_Declaration* find_attribute (Ast_Struct_Type* struct_type, const char* name) {
        return Ast_Utils::find_local_declaration(&struct_type->scope, name);
    }
};