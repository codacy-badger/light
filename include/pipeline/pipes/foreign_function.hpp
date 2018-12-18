#pragma once

#include "pipeline/pipe.hpp"
#include "report.hpp"

struct Foreign_Function : Pipe {
	Foreign_Function () { this->pipe_name = "Foreign_Function"; }

	void handle (Ast_Declaration** decl_ptr) {
		auto decl = (*decl_ptr);

		auto note = decl->remove_note("foreign");
		if (note) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
				auto func = static_cast<Ast_Function*>(decl->expression);

				auto module_name = note->get_string_parameter(0);
				if (module_name) {
					func->foreign_module_name = module_name;
					if (note->arguments.size() == 2) {
						func->foreign_function_name = note->get_string_parameter(1);
					} else func->foreign_function_name = func->name;

					auto module = os_get_module(func->foreign_module_name);
					if (module) {
						func->foreign_function_pointer = os_get_function(module, func->foreign_function_name);
						if (!func->foreign_function_pointer) {
							ERROR_STOP(func, "Function '%s' not found in module '%s'!",
								func->foreign_function_name, func->foreign_module_name);
						}
					} else ERROR_STOP(func, "Module '%s' not found!", func->foreign_module_name);
				} else ERROR_STOP(note, "foreign note parameter is not a string!");
			}
		}
	}
};
