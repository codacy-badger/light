#include "parser/pipe/foreign_function.hpp"

#include <string.h>

#include "compiler.hpp"

Ast_Note* remove_foreign_note (Ast_Declaration* decl) {
	if (decl->notes.size() > 0) {
		auto it = decl->notes.begin();
		while (it != decl->notes.end()) {

			if (strcmp((*it)->name, "foreign") == 0) {
				decl->notes.erase(it);
				return (*it);
			} else it++;
		}
	}
	return NULL;
}

void Foreign_Function::on_statement(Ast_Statement* stm) {
	if (stm->stm_type == AST_STATEMENT_DECLARATION) {
		auto decl = static_cast<Ast_Declaration*>(stm);
		if (!decl->expression && decl->notes.size() > 0) {
			if (decl->type->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
				auto type_def = static_cast<Ast_Type_Definition*>(decl->type);
				if (type_def->typedef_type == AST_TYPEDEF_FUNCTION) {
					auto fn_type = static_cast<Ast_Function_Type*>(type_def);

					auto note = remove_foreign_note(decl);
					if (note) {
						auto fn = new Ast_Function();
						fn->name = decl->name;
						fn->type = fn_type;

						if (!note->arguments || note->arguments->args.size() != 1) {
							Light_Compiler::inst->error_stop(note, "foreign notes must have 1 (string) parameter!");
						}

						auto exp = note->arguments->args[0];
						if (exp->exp_type == AST_EXPRESSION_LITERAL) {
							auto lit = static_cast<Ast_Literal*>(exp);
							if (lit->literal_type == AST_LITERAL_STRING) {
								fn->foreign_module_name = lit->string_value;
								delete note;
							} else Light_Compiler::inst->error_stop(note, "foreign note parameter is not a string!");
						} else Light_Compiler::inst->error_stop(note, "foreign notes parameter is not a literal!");

						decl->expression = fn;
					}
				}
			}
		}
	}
	this->to_next(stm);
}
