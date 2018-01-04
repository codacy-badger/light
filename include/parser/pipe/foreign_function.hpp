#pragma once

#include "parser/pipes.hpp"

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
		} else report_error_stop(&exp->location, "foreign note parameter is not a string!");
	} else report_error_stop(&exp->location, "foreign notes parameter is not a literal!");
	return NULL;
}

struct Foreign_Function : Pipe {

	void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		auto note = remove_foreign_note(decl);
		if (note) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
				auto func = static_cast<Ast_Function*>(decl->expression);

				auto exp = note->arguments[0];
				auto module_name = extract_string_parameter(exp);
				if (module_name) {
					func->foreign_module_name = module_name;
					if (note->arguments.size() == 2) {
						exp = note->arguments[1];
						func->foreign_function_name = extract_string_parameter(exp);
					} else func->foreign_function_name = func->name;
				}
			}
		}
	}
};
