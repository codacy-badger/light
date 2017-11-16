#include "bytecode/bytecode_runner.hpp"

#include <stdio.h>
#include <windows.h>

#include "compiler.hpp"

void dummy (uint32_t number) {
	printf("  --  NUMBER: %d\n", number);
}

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

void Bytecode_Runner::on_statement (Ast_Statement* stm) {
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);

        if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(decl->expression);
			auto ret_ty = static_cast<Ast_Type_Definition*>(func->type->return_type);

            auto run_note = remove_run_note(decl);
            if (run_note) {
				if (func->foreign_module_name) {
					dcMode(this->vm, DC_CALL_C_X64_WIN64);
					dcReset(this->vm);
					for (auto exp : run_note->arguments->args) {
						if (exp->exp_type == AST_EXPRESSION_LITERAL) {
							auto lit = static_cast<Ast_Literal*>(exp);
							switch (lit->literal_type) {
								case AST_LITERAL_UNSIGNED_INT: dcArgInt(this->vm, lit->uint_value); break;
								case AST_LITERAL_SIGNED_INT: dcArgInt(this->vm, lit->int_value); break;
								case AST_LITERAL_DECIMAL: dcArgDouble(this->vm, lit->int_value); break;
								case AST_LITERAL_STRING: dcArgPointer(this->vm, lit->string_value); break;
							}
						} else {
							Light_Compiler::inst->error_stop(run_note, "Run notes can only have literal arguments!");
						}
					}

					HMODULE module = LoadLibrary(func->foreign_module_name);
					DCpointer fn_ptr = (DCpointer) GetProcAddress(module, func->name);
					switch (ret_ty->typedef_type) {
						case AST_TYPEDEF_STRUCT: {
							Light_Compiler::inst->error_stop(run_note, "Struct return types not yet supported!");
						}
						case AST_TYPEDEF_POINTER: {
							void* ret = dcCallPointer(this->vm, fn_ptr);

							const char* msg = "\nDYNCALL is super awesome!\n\n";
							WriteFile(ret, msg, strlen(msg), NULL, NULL);
						}
					}
				} else {
	                for (auto inst : func->bytecode) {
	                    Light_Compiler::inst->interp->run(inst);
	                }
	                Light_Compiler::inst->interp->dump();
				}
				// TODO: replace function call by output (?)
            }
        }
    }
    this->to_next(stm);
}
