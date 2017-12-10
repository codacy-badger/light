#include "parser/pipe/foreign_function.hpp"

#include <string.h>

#include "compiler.hpp"

Ast_Note* remove_foreign_note (Ast_Declaration* decl) {
	if (decl->notes.size() > 0) {
		auto it = decl->notes.begin();
		while (it != decl->notes.end()) {

			if (strcmp((*it)->name, "foreign") == 0) {
				auto output = (*it);
				decl->notes.erase(it);
				return output;
			} else it++;
		}
	}
	return NULL;
}

char* extract_string_parameter (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_LITERAL) {
		auto lit = static_cast<Ast_Literal*>(exp);
		if (lit->literal_type == AST_LITERAL_STRING) {
			return lit->string_value;
		} else Light_Compiler::inst->error_stop(exp, "foreign note parameter is not a string!");
	} else Light_Compiler::inst->error_stop(exp, "foreign notes parameter is not a literal!");
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
						ast_copy_location_info(fn, stm);
						fn->name = decl->name;
						fn->type = fn_type;

						if (!note->arguments || (note->arguments->values.size() != 1
							&& note->arguments->values.size() != 2)) {
							Light_Compiler::inst->error_stop(note, "foreign notes must have 1 or 2 (string) parameter!");
						}

						auto exp = note->arguments->values[0];
						auto module_name = extract_string_parameter(exp);
						if (module_name) {
							fn->foreign_module_name = module_name;
							if (note->arguments->values.size() == 2) {
								exp = note->arguments->values[1];
								fn->foreign_function_name = extract_string_parameter(exp);
							} else fn->foreign_function_name = fn->name;
						}
						delete note;

						decl->expression = fn;
					}
				}
			}
		}
	}
	this->to_next(stm);
}
