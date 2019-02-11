#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"
#include "utils/string_map.hpp"

#include <map>
#include <vector>

struct Resolve_Idents : Compiler_Pipe<Ast_Scope*, Ast_Statement*>, Ast_Navigator {

    Ast_Scope* current_file_scope = NULL;
    Ast_Statement* current_global_statement = NULL;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Scope* file_scope) {
        this->current_file_scope = file_scope;
        for (auto stm : file_scope->statements) {
            this->current_global_statement = stm;
            Ast_Navigator::ast_handle(stm);
        }

        for (auto stm : file_scope->statements) {
            this->push_out(stm);
        }
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            ident->declaration = this->current_file_scope->find_declaration(ident->name, true, true);
            if (!ident->declaration) {
                this->print_error(ident, "Identifier not found: '%s'", ident->name);
            }
        }
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
                        auto import = static_cast<Ast_Import*>(ident->declaration->expression);
                        auto attr = static_cast<Ast_Ident*>(binop->rhs);
                        attr->declaration = import->file_scope->find_declaration(attr->name, false, false);
                    }
                }
            }
        }
    }
};
