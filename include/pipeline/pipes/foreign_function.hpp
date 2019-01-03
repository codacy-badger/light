#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

#include "report.hpp"

struct Foreign_Function : Scoped_Statement_Pipe {
	Foreign_Function () { this->pipe_name = "Foreign_Function"; }

	void handle (Ast_Directive_Foreign** foreign_ptr) {
		auto foreign = (*foreign_ptr);

		auto stm_location = this->delete_current_statement();
		for (auto decl : foreign->declarations) {
			auto func = static_cast<Ast_Function*>(decl->expression);

			// @TODO @Optimize NO NO NO we shouldn't get the function pointer
			// unless we're going to call the function for sure, so this
			// should be wrapped around lazy loading
			auto module = os_get_module(func->foreign_module_name);
			if (module) {
				func->foreign_function_pointer = os_get_function(module, func->foreign_function_name);
				if (!func->foreign_function_pointer) {
					ERROR_STOP(func, "Function '%s' not found in module '%s'!",
						func->foreign_function_name, func->foreign_module_name);
				}
			} else ERROR_STOP(func, "Module '%s' not found!", func->foreign_module_name);

			stm_location = this->current_scope->statements.insert(stm_location, decl);
		}
	}
};
