#pragma once

#include "pipeline/pipe.hpp"

#include "ast/ast_cloner.hpp"

struct Constant_Propagation : Pipe {

    Constant_Propagation () { this->pipe_name = "Constant_Propagation"; }

    void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (ident->declaration && ident->declaration->is_constant) {
		    auto _addr = reinterpret_cast<Ast_Expression**>(ident_ptr);
			auto exp = (*ident_ptr)->declaration->expression;
			if (exp->exp_type != AST_EXPRESSION_TYPE_INSTANCE
					&& exp->exp_type != AST_EXPRESSION_FUNCTION) {
				(*_addr) = Ast_Cloner::clone(exp);
			} else (*_addr) = exp;
        }
	}
};
