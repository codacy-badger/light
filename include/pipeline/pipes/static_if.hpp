#pragma once

#include "pipeline/pipe.hpp"

struct Static_If : Pipe {
    PIPE_NAME(Static_If)

	Ast_Statement* current_stm = NULL;
    Ast_Scope* current_scope = NULL;

    void handle (Ast_Directive_If** if_ptr) {
		auto if_directive = (*if_ptr);

        auto location = get_current_stm_location();
		location = this->current_scope->statements.erase(location);

		auto _if = static_cast<Ast_If*>(if_directive->stm_if);
        if (_if->condition->exp_type == AST_EXPRESSION_LITERAL) {
            auto lit = static_cast<Ast_Literal*>(_if->condition);

            if (lit->as_boolean()) {
                auto scope = static_cast<Ast_Scope*>(_if->then_statement);
                this->pipeline->process(scope);

                for (auto stm : scope->statements) {
                    location = this->current_scope->statements.insert(location, stm);
                }
            } else if (_if->else_statement) {
                auto scope = static_cast<Ast_Scope*>(_if->else_statement);
                this->pipeline->process(scope);

                for (auto stm : scope->statements) {
                    location = this->current_scope->statements.insert(location, stm);
                }
            }
        }
	}

    void handle (Ast_Scope** block_ptr) {
        auto tmp = this->current_scope;
        this->current_scope = (*block_ptr);
        Pipe::handle(block_ptr);
        this->current_scope = tmp;
    }

    void handle (Ast_Statement** stm_ptr) {
        auto tmp = this->current_stm;
        this->current_stm = (*stm_ptr);
        Pipe::handle(stm_ptr);
        this->current_stm = tmp;
    }

    vector<Ast_Statement*>::iterator get_current_stm_location () {
        return find(this->current_scope->statements.begin(),
            this->current_scope->statements.end(), this->current_stm);
    }
};
