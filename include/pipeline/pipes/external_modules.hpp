#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

#include "compiler.hpp"

struct External_Modules : Scoped_Statement_Pipe {
	PIPE_NAME(External_Modules)

    void handle (Ast_Directive_Include** include_ptr) {
		auto include = (*include_ptr);

		auto location = this->get_current_stm_location();
		location = this->current_scope->statements.erase(location);

		this->total_time += os_time_user_stop(this->start_time);
		auto scope = Compiler::instance->modules->get_module(include->absolute_path);
		this->start_time = os_get_user_time();

		for (auto stm : scope->statements) {
			if (stm->stm_type == AST_STATEMENT_DECLARATION) {
				auto decl = static_cast<Ast_Declaration*>(stm);

				auto other_decl = this->current_scope->find_declaration(decl->name);
				if (other_decl) {
					ERROR(other_decl, "Re-declaration of \"%s\"", decl->name);
					ERROR_STOP(decl, "Original declaration here");
				}
			}

			location = this->current_scope->statements.insert(location, stm);
		}
    }

    void handle (Ast_Directive_Import** import_ptr) {
		auto import = (*import_ptr);

		this->current_scope->statements.erase(get_current_stm_location());

		this->total_time += os_time_user_stop(this->start_time);
		auto scope = Compiler::instance->modules->get_module(import->absolute_path);
		this->start_time = os_get_user_time();

		this->current_scope->module_scopes.push_back(scope);
    }
};
