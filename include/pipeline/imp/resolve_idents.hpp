#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include <map>

struct Resolve_Idents : Compiler_Pipe<Ast_Scope*, Ast_Statement*>, Ast_Ref_Navigator {

    std::map<Ast_Scope*, std::map<Ast_Statement*, std::vector<Ast_Ident*>>> unresolved_idents;
    Ast_Statement* current_global_statement = NULL;
    Ast_Scope* current_file_scope = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Scope* file_scope) {
        this->current_file_scope = file_scope;
        for (auto& stm : file_scope->statements) {
            this->current_global_statement = stm;
            this->ast_handle(&stm);
        }

        auto it = this->unresolved_idents.find(file_scope);
        if (it == this->unresolved_idents.end()) {
            for (auto stm : file_scope->statements) {
                this->push_out(stm);
            }
        } else {
            for (auto stm : file_scope->statements) {
                auto it2 = this->unresolved_idents[file_scope].find(stm);
                if (it2 == this->unresolved_idents[file_scope].end()) {
                    this->push_out(stm);
                }
            }
        }
    }

    bool resolve_ident (Ast_Expression**, Ast_Ident* ident, Ast_Declaration* decl) {
        /*if (decl->is_constant && decl->expression) {
            auto exp_type = decl->expression->exp_type;
            if (exp_type == AST_EXPRESSION_FUNCTION || exp_type == AST_EXPRESSION_TYPE) {
                (*exp_ptr) = decl->expression;
                return true;
            }
        }*/
        ident->declaration = decl;
        return false;
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            auto decl = this->current_file_scope->find_declaration(ident->name, true, true);
            if (decl) {
                this->resolve_ident((Ast_Expression**) ident_ptr, ident, decl);
            } else {
                this->unresolved_idents[this->current_file_scope][this->current_global_statement].push_back(ident);
            }
        }
    }

    void ast_handle (Ast_Binary** binop_ptr) {
        auto binop = (*binop_ptr);

        if (binop->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Ref_Navigator::ast_handle(binop_ptr);
        } else {
            Ast_Ref_Navigator::ast_handle(&binop->lhs);
            if (binop->lhs->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(binop->lhs);
                if (ident->declaration && ident->declaration->expression) {
                    if (ident->declaration->expression->exp_type == AST_EXPRESSION_IMPORT) {
                        auto import = static_cast<Ast_Import*>(ident->declaration->expression);
                        auto attr = static_cast<Ast_Ident*>(binop->rhs);
                        auto decl = import->file_scope->find_declaration(attr->name, false, false);

                        // INFO: if the resolved identifier cannot be replaced by the declaration's
                        // value, we make sure that the namespace access expression gets replaced
                        // by the ident, so we remove the namespace access completely.
                        auto is_replaced = this->resolve_ident((Ast_Expression**) binop_ptr, attr, decl);
                        if (!is_replaced) (*binop_ptr) = (Ast_Binary*) attr;
                    }
                }
            }
        }
    }

    void ast_handle (Ast_Statement** stm_ptr) {
        Ast_Ref_Navigator::ast_handle(stm_ptr);
        (*stm_ptr)->stm_flags |= STM_FLAG_IDENTS_RESOLVED;
    }

    void shutdown () {
        if (!this->unresolved_idents.empty()) {
            for (auto entry1 : this->unresolved_idents) {
                this->print_error("Error found in '%s'...", entry1.first->location.filename);
                for (auto entry2 : entry1.second) {
                    for (auto ident : entry2.second) {
                        this->print_error(ident, "  ...Identifier not found: '%s'", ident->name);
                    }
                }
            }
        }
    }
};
