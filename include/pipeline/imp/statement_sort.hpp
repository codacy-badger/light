#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

struct Statement_Sort : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    bool ident_with_unresolved_declaration_found = false;

    Statement_Sort() : Compiler_Pipe("Statement Sort") {}

    void handle (Ast_Statement* global_statement) {
        this->ident_with_unresolved_declaration_found = false;
        Ast_Navigator::ast_handle(global_statement);

        if (this->ident_with_unresolved_declaration_found) {
            this->requeue(global_statement);
        } else this->push_out(global_statement);
    }

    void ast_handle (Ast_Ident* ident) {
        if (this->ident_with_unresolved_declaration_found) return;

        assert(ident->declaration);
        auto flag = ident->declaration->stm_flags & STM_FLAG_IDENTS_RESOLVED;
        this->ident_with_unresolved_declaration_found = !flag;
    }

    void ast_handle (Ast_Binary* binop) {
        if (binop->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Navigator::ast_handle(binop);
        } else {
            Ast_Navigator::ast_handle(binop->lhs);
            if (binop->lhs->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(binop->lhs);
                if (ident->declaration && ident->declaration->expression) {
                    if (ident->declaration->expression->exp_type == AST_EXPRESSION_IMPORT) {
                        Ast_Navigator::ast_handle(binop->rhs);
                    }
                }
            }
        }
    }

    void ast_handle (Ast_Unary* unary) {
        if (unary->unary_op != AST_UNARY_REFERENCE) {
            Ast_Navigator::ast_handle(unary);
        }
    }
};
