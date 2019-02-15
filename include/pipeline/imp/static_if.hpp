#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Static_If : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Ast_Scope* current_scope = NULL;
    Async_Queue<Ast_Statement*>* flow_back = NULL;

    Static_If(Async_Queue<Ast_Statement*>* flow_back) : Compiler_Pipe("Static If") {
        this->flow_back = flow_back;
    }

    void handle (Ast_Statement* global_statement) {
        this->current_scope = global_statement->parent_scope;
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Static_If* static_if) {
        auto _if = static_if->stm_if;

        auto condition_value = this->get_value_as_bool(_if->condition);
        if (condition_value) {
            this->resolve_static_if(static_if, _if->then_body);
        } else if (_if->else_body) {
            this->resolve_static_if(static_if, _if->else_body);
        }
    }

    void resolve_static_if (Ast_Static_If* static_if, Ast_Scope* scope_to_merge) {
        static_if->parent_scope->add(static_if->parent_scope->find(static_if), scope_to_merge);
        for (auto stm : scope_to_merge->statements) {
            flow_back->push(stm);
        }
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;
        Ast_Navigator::ast_handle(scope);
        this->current_scope = tmp;
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
            this->context->error(exp, "Static IF condition can only be literal or pre-declared constant");
            return false;
        }
    }
};
