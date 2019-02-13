#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

struct Resolve_Idents : Compiler_Pipe<Ast_Scope*, Ast_Statement*>, Ast_Ref_Navigator {

    Ast_Scope* current_file_scope = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Scope* file_scope) {
        this->current_file_scope = file_scope;
        for (auto& stm : file_scope->statements) {
            this->ast_handle(&stm);
        }

        for (auto stm : file_scope->statements) {
            this->push_out(stm);
        }
    }

    bool resolve_ident (Ast_Expression** exp_ptr, Ast_Ident* ident, Ast_Declaration* decl) {
        if (decl->is_constant && decl->expression) {
            auto exp_type = decl->expression->exp_type;
            if (exp_type == AST_EXPRESSION_FUNCTION || exp_type == AST_EXPRESSION_TYPE) {
                (*exp_ptr) = decl->expression;
                return true;
            }
        }
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
                this->print_error(ident, "Identifier not found: '%s'", ident->name);
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
};
