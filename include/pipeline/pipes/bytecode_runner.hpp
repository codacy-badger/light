#pragma once

#include "pipeline/pipe.hpp"

#include "compiler.hpp"

size_t run_function (Ast_Function* func, Ast_Note* run_note) {
	if (func->is_native()) {
		report_error_and_stop(&run_note->location, "#run can't go on foreign functions (for now)");
	} else {
		for (size_t i = 0; i < run_note->arguments.size(); i++) {
			auto exp = run_note->arguments[i];
			if (exp->exp_type == AST_EXPRESSION_LITERAL) {
				auto lit = static_cast<Ast_Literal*>(exp);
				auto reg = Compiler::inst->interp->registers[i];
				memcpy(reg, &lit->int_value, INTERP_REGISTER_SIZE);
			} else {
				report_error_and_stop(&run_note->location, "#run can only have literal arguments!");
			}
		}
		Compiler::inst->interp->run(func);
	}
	return NULL;
}

struct Bytecode_Runner : Pipe {
	PIPE_NAME(Bytecode_Runner)

    void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		if (decl->expression) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
				auto func = static_cast<Ast_Function*>(decl->expression);
				auto run_note = decl->remove_note("run");
				if (run_note) run_function(func, run_note);
			}
		}
	}
};
