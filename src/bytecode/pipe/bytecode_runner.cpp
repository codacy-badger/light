#include "bytecode/pipe/bytecode_runner.hpp"

#include "compiler.hpp"

Ast_Note* remove_run_note (Ast_Declaration* decl) {
	if (decl->notes.size() > 0) {
		auto it = decl->notes.begin();
		while (it != decl->notes.end()) {
			if (strcmp((*it)->name, "run") == 0) {
				auto output = (*it);
				decl->notes.erase(it);
				return output;
			} else it++;
		}
	}
	return NULL;
}

size_t run_function (Ast_Function* func, Ast_Note* run_note) {
	if (func->foreign_module_name) {
		report_error_stop(&run_note->location, "#run can't go on foreign functions (for now)");
	} else {
		if (run_note->arguments) {
			for (size_t i = 0; i < run_note->arguments->values.size(); i++) {
				auto exp = run_note->arguments->values[i];
				if (exp->exp_type == AST_EXPRESSION_LITERAL) {
					auto lit = static_cast<Ast_Literal*>(exp);
					auto reg = g_compiler->interp->registers[i];
					memcpy(reg, &lit->int_value, INTERP_REGISTER_SIZE);
				} else {
					report_error_stop(&run_note->location, "#run can only have literal arguments!");
				}
			}
		}
		g_compiler->interp->run(func);
	}
	return NULL;
}

void Bytecode_Runner::on_statement (Ast_Statement* stm) {
	this->run(stm);
    this->to_next(stm);
}

void Bytecode_Runner::run (Ast_Statement* stm) {
	switch (stm->stm_type) {
		case AST_STATEMENT_DECLARATION: {
			this->run(static_cast<Ast_Declaration*>(stm));
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			// TODO: allow #run on expressions, replace expression by result
			break;
		}
		default: break;
	}
}

void Bytecode_Runner::run (Ast_Declaration* decl) {
	this->run(decl->type);
	if (decl->expression) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			auto func = static_cast<Ast_Function*>(decl->expression);
			auto run_note = remove_run_note(decl);
			if (run_note) run_function(func, run_note);
		}
		this->run(decl->expression);
	}
}
