#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"
#include "utils/string_map.hpp"

#include <map>
#include <vector>

struct Resolve_Idents : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {

    std::map<Ast_Statement*, std::vector<Ast_Ident*>> unresolved_idents;
    String_Map<Ast_Declaration*> unresolved_decls;
    Ast_Statement* current_statement = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        this->current_statement = global_statement;
        Ast_Navigator::ast_handle(global_statement);

        auto it = this->unresolved_idents.find(global_statement);
        if (it == this->unresolved_idents.end()) {
            this->on_resolved(global_statement);
        }
    }

    void on_resolved (Ast_Statement* stm) {
        this->push_out(stm);
        if (this->current_statement->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            this->try_resolve(decl);
        }
    }

    void try_resolve (Ast_Declaration* decl) {
        // TODO: find declarations that import the file_scope of the current declaration
        // and try to resolved the unresolved symbol of those... seems too convoluted
        std::vector<Ast_Statement*> resolved;
        for (auto& entry : this->unresolved_idents) {
            for (size_t i = 0; i < entry.second.size();) {
                auto ident_to_resolve = entry.second[i];
                if (strcmp(ident_to_resolve->name, decl->name) == 0) {
                    ident_to_resolve->declaration = decl;
                    entry.second.erase(entry.second.begin() + i);
                } else i++;
            }

            if (entry.second.size() == 0) {
                resolved.push_back(entry.first);
            }
        }

        for (auto stm : resolved) {
            this->remove_unresolved(stm);
            this->on_resolved(stm);
        }
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            ident->declaration = this->find_declaration(ident);
            if (!ident->declaration) this->add_unresolved(ident);
        }
    }

    Ast_Declaration* find_declaration (Ast_Ident* ident) {
        auto decl = ident->scope->find_declaration(ident->name, true, true);
        if (!decl) {
            if (this->unresolved_decls.contains(ident->name)) {
                return this->unresolved_decls[ident->name];
            } else return NULL;
        } else return decl;
    }

    void add_unresolved (Ast_Ident* ident) {
        this->unresolved_idents[this->current_statement].push_back(ident);
        if (this->current_statement->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(this->current_statement);
            this->unresolved_decls[decl->name] = decl;
        }
    }

    void remove_unresolved (Ast_Statement* stm) {
        this->unresolved_idents.erase(stm);
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            this->unresolved_decls.erase(decl->name);
        }
    }

    void ast_handle (Ast_Binary* binop) {
        if (binop->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Navigator::ast_handle(binop);
        }
    }

    void shutdown() {
        if (!this->unresolved_idents.empty()) {
            printf("PRINT ERRORS!\n");
        }
    }
};
