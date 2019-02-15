#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include <map>
#include <vector>

struct Resolve_Idents : Compiler_Pipe<Ast_Statement*>, Ast_Ref_Navigator {
    String_Map<std::vector<Ast_Statement*>> depending_statements;

    std::vector<Ast_Ident**> unresolved_idents;
    Ast_Scope* current_scope = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        this->current_scope = global_statement->parent_scope;

        this->unresolved_idents.clear();
        Ast_Ref_Navigator::ast_handle(&global_statement);

        if (this->unresolved_idents.empty()) {
            this->push_out(global_statement);
            if (global_statement->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(global_statement);
                if (this->depending_statements.contains(decl->name)) {
                    auto deps = &(this->depending_statements[decl->name]);
                    for (auto stm : *deps) {
                        this->requeue(stm);
                    }
                    deps->clear();
                }
            }
        } else {
            auto ident_name = (*(this->unresolved_idents[0]))->name;
            this->depending_statements[ident_name].push_back(global_statement);
        }
    }

    bool resolve_ident (Ast_Expression** exp_ptr, Ast_Ident* ident, Ast_Declaration* decl) {
        if (decl->is_constant && decl->expression) {
            auto exp_type = decl->expression->exp_type;
            if (exp_type == AST_EXPRESSION_FUNCTION || exp_type == AST_EXPRESSION_TYPE) {
                (*exp_ptr) = decl->expression;
                Ast_Ref_Navigator::ast_handle(exp_ptr);
                return true;
            }
        }
        ident->declaration = decl;
        Ast_Ref_Navigator::ast_handle(&ident->declaration);
        return false;
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            auto decl = this->current_scope->find_declaration(ident->name, true, true);
            if (decl) {
                this->resolve_ident((Ast_Expression**) ident_ptr, ident, decl);
            } else {
                this->unresolved_idents.push_back(ident_ptr);
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
                        auto attr = static_cast<Ast_Ident*>(binop->rhs);

                        auto import = static_cast<Ast_Import*>(ident->declaration->expression);
                        assert(import->file_scope);

                        auto decl = import->file_scope->find_declaration(attr->name, false, false);
                        if (decl) {
                            // INFO: if the resolved identifier cannot be replaced by the declaration's
                            // value, we make sure that the namespace access expression gets replaced
                            // by the ident, so we remove the namespace access completely.
                            auto is_replaced = this->resolve_ident((Ast_Expression**) binop_ptr, attr, decl);
                            if (!is_replaced) (*binop_ptr) = (Ast_Binary*) attr;
                        } else {
                            this->unresolved_idents.push_back((Ast_Ident**) &binop->rhs);
                        }
                    }
                }
            }
        }
    }

    void ast_handle (Ast_Scope** scope_ptr) {
        auto tmp = this->current_scope;
        this->current_scope = (*scope_ptr);

        Ast_Ref_Navigator::ast_handle(scope_ptr);

        this->current_scope = tmp;
    }

    void shutdown () {
        if (!this->input_queue.empty()) {
            while (!this->input_queue.empty()) {
                auto stm = this->input_queue.pop();

                this->unresolved_idents.clear();
                Ast_Ref_Navigator::ast_handle(&stm);

                for (auto ident_ptr : this->unresolved_idents) {
                    auto ident = (*ident_ptr);

                    this->error(ident, "Unresolved identifier: '%s'", ident->name);
                }
            }
        }
    }
};
