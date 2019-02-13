#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Static_If : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Async_Queue<Ast_Scope*>* flowback_queue = NULL;

    Static_If(Async_Queue<Ast_Scope*>* flowback_queue) : Compiler_Pipe("Static If") {
        this->flowback_queue = flowback_queue;
    }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Static_If* static_if) {
        auto _if = static_if->stm_if;

        auto condition_value = this->get_value_as_bool(_if->condition);
        if (condition_value) {
            this->flowback_queue->push(_if->then_body);
        } else if (_if->else_body) {
            this->flowback_queue->push(_if->else_body);
        }
    }

    bool get_value_as_bool (Ast_Expression* exp) {
        if (exp->exp_type == AST_EXPRESSION_LITERAL) {
            auto literal = static_cast<Ast_Literal*>(exp);
            return literal->uint_value;
        } else if (exp->exp_type == AST_EXPRESSION_IDENT) {
            auto ident = static_cast<Ast_Ident*>(exp);
            assert(ident->declaration);
            assert(ident->declaration->is_constant);
            return this->get_value_as_bool(ident->declaration->expression);
        } else {
            this->print_error(exp, "Static IF condition can only be literal or constant");
            return false;
        }
    }
};
