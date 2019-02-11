#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"
#include "utils/string_map.hpp"

#include <map>
#include <vector>

struct Resolve_Idents : Compiler_Pipe<Ast_Scope*, Ast_Statement*>, Ast_Navigator {

    std::map<Ast_Scope*, std::map<Ast_Statement*, std::vector<Ast_Ident*>>> unresolved_idents;

    Ast_Scope* current_file_scope = NULL;
    Ast_Statement* current_global_statement = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Scope* file_scope) {
        this->current_file_scope = file_scope;
        for (auto stm : file_scope->statements) {
            this->current_global_statement = stm;
            Ast_Navigator::ast_handle(stm);
        }

        auto it1 = this->unresolved_idents.find(file_scope);
        if (it1 == this->unresolved_idents.end()) {
            for (auto stm : file_scope->statements) {
                this->on_resolved(file_scope, stm);
            }
        } else {
            for (auto stm : file_scope->statements) {
                auto it2 = it1->second.find(stm);
                if (it2 == it1->second.end()) {
                    this->on_resolved(file_scope, stm);
                }
            }
        }
    }

    void on_resolved (Ast_Scope* scope, Ast_Statement* stm) {
        this->push_out(stm);
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            this->try_resolve(scope, decl);
        }
    }

    void try_resolve (Ast_Scope* scope, Ast_Declaration* decl) {
        std::map<Ast_Scope*, std::vector<Ast_Statement*>> resolved;

        for (auto& entry1 : this->unresolved_idents) {
            if (entry1.first->has_import(scope)) {
                for (auto& entry2 : entry1.second) {
                    for (size_t i = 0; i < entry2.second.size();) {
                        auto ident_to_resolve = entry2.second[i];
                        if (strcmp(ident_to_resolve->name, decl->name) == 0) {
                            ident_to_resolve->declaration = decl;
                            entry2.second.erase(entry2.second.begin() + i);
                        } else i++;
                    }

                    if (entry2.second.size() == 0) {
                        resolved[entry1.first].push_back(entry2.first);
                    }
                }
            }
        }

        for (auto entry : resolved) {
            for (auto stm : entry.second) {
                this->remove_unresolved(entry.first, stm);
                this->on_resolved(entry.first, stm);
            }
        }
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            ident->declaration = this->find_declaration(this->current_file_scope, ident);
            if (!ident->declaration) {
                this->add_unresolved(this->current_file_scope, ident);
            }
        }
    }

    Ast_Declaration* find_declaration (Ast_Scope* scope, Ast_Ident* ident) {
        auto decl = scope->find_declaration(ident->name, true, true);
        if (!decl) {
            decl = this->get_unresolved_decl(scope, ident->name);
        }
        return decl;
    }

    void add_unresolved (Ast_Scope* scope, Ast_Ident* ident) {
        this->unresolved_idents[scope][this->current_global_statement].push_back(ident);
    }

    void remove_unresolved (Ast_Scope* scope, Ast_Statement* stm) {
        this->unresolved_idents[scope].erase(stm);
        if (this->unresolved_idents[scope].size() == 0) {
            this->unresolved_idents.erase(scope);
        }
    }

    Ast_Declaration* get_unresolved_decl (Ast_Scope* scope, const char* _name) {
        for (auto entry : this->unresolved_idents[scope]) {
            if (entry.first->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(entry.first);
                if (strcmp(decl->name, _name) == 0) {
                    return decl;
                }
            }
        }
        return NULL;
    }

    void ast_handle (Ast_Binary* binop) {
        if (binop->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Navigator::ast_handle(binop);
        } else Ast_Navigator::ast_handle(binop->lhs);
    }

    void shutdown() {
        for (auto entry1 : this->unresolved_idents) {
            assert(entry1.second.size() > 0);

            this->print_error("Unresolved identifiers in module '%s'...", entry1.first->location.filename);
            for (auto entry2 : entry1.second) {
                if (entry2.first->stm_type == AST_STATEMENT_DECLARATION) {
                    auto decl = static_cast<Ast_Declaration*>(entry2.first);
                    this->print_error("\t...for global declaration of '%s'...", decl->name);
                } else this->print_error("\t...for global statement...");
                for (auto ident : entry2.second) {
                    this->print_error("\t\tUnresolved identifier: '%s' @", ident->name);
                }
            }
        }
    }
};
