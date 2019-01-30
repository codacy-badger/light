#pragma once

#include "steps/sync_pipe.hpp"

#include "utils/ast_navigator.hpp"

struct Constant_If_Step : Sync_Pipe, Ast_Navigator {

    Constant_If_Step() : Sync_Pipe("Constant If") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        Ast_Navigator::ast_handle(stm);
        this->pipe_out(in);
    }

    void ast_handle (Ast_If* _if) {
        auto current_scope = this->current_scope();
        if (current_scope->is_global()) {
            this->remove_current_statement = true;

            if (_if->condition->exp_type == AST_EXPRESSION_LITERAL) {
                auto literal = static_cast<Ast_Literal*>(_if->condition);

                auto it = this->get_current_stm_location();
                if (literal->uint_value) {
                    current_scope->add(it, _if->then_body);
                } else if (_if->else_body) {
                    current_scope->add(it, _if->else_body);
                }
            } else {
                printf("ERROR: global scope IFs must have a constant condition");
            }
        }
    }
};
