#include "bytecode/bytecode_runner.hpp"

#include <stdio.h>

#include "compiler.hpp"

Ast_Note* remove_run_note (Ast_Declaration* decl) {
	if (decl->notes.size() > 0) {
		auto it = decl->notes.begin();
		while (it != decl->notes.end()) {
			if (strcmp((*it)->name, "run") == 0) {
				decl->notes.erase(it);
				return (*it);
			} else it++;
		}
	}
	return NULL;
}

void Bytecode_Runner::on_statement (Ast_Statement* stm) {
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);

        if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
            auto fn = static_cast<Ast_Function*>(decl->expression);

            auto run_note = remove_run_note(decl);
            if (run_note) {
                printf(" !! About to run bytecode !!\n");
                for (auto inst : fn->bytecode) {
                    Light_Compiler::inst->interp->run(inst);
                }
                Light_Compiler::inst->interp->dump();
                // TODO: replace function call by output (?)
            }
        }
    }
    this->to_next(stm);
}
