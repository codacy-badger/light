#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

struct Static_If : Scoped_Statement_Pipe {
    PIPE_NAME(Static_If)

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
};
