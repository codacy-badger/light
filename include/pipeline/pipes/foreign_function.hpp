#pragma once

#include "pipeline/pipe.hpp"
#include "report.hpp"

char* extract_string_parameter (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_LITERAL) {
		auto lit = static_cast<Ast_Literal*>(exp);
		if (lit->literal_type == AST_LITERAL_STRING) {
			return lit->string_value;
		} else ERROR_STOP(exp, "foreign note parameter is not a string!");
	} else ERROR_STOP(exp, "foreign notes parameter is not a literal!");
	return NULL;
}

struct Foreign_Function : Pipe {
	PIPE_NAME(Foreign_Function)

	void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		auto note = decl->remove_note("foreign");
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

					auto module = os_get_module(func->foreign_module_name);
					if (module) {
						func->foreign_function_pointer = os_get_function(module, func->foreign_function_name);
						if (!func->foreign_function_pointer) {
							ERROR_STOP(func, "Function '%s' not found in module '%s'!",
								func->foreign_function_name, func->foreign_module_name);
						}
					} else ERROR_STOP(func, "Module '%s' not found!", func->foreign_module_name);
				}
			}
		}
	}
};