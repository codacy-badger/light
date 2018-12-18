#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

#include "compiler.hpp"

struct External_Modules : Scoped_Statement_Pipe {
	External_Modules () { this->pipe_name = "External_Modules"; }

    void handle (Ast_Directive_Include** include_ptr) {
		auto include = (*include_ptr);

		auto location = this->delete_current_statement();
		find_existing_absolute_path(&include->location, include->path, include->absolute_path);

		auto scope = this->get_external_module(include->absolute_path);
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

		this->delete_current_statement();
		find_existing_absolute_path(&import->location, import->path, import->absolute_path);

		auto scope = this->get_external_module(import->absolute_path);
		this->current_scope->module_scopes.push_back(scope);
    }

	Ast_Scope* get_external_module (char* absolute_path) {
		this->total_time += os_time_user_stop(this->start_time);
		auto scope = Compiler::inst->modules->get_module(absolute_path);
		this->start_time = os_get_user_time();
		return scope;
	}

	void find_existing_absolute_path (Location* location,
			const char* relative_path, char* absolute_path) {
		this->get_relative_to_current_file(relative_path, absolute_path);
		if (os_check_file_exists(absolute_path)) return;

		this->get_relative_to_main_file(relative_path, absolute_path);
		if (os_check_file_exists(absolute_path)) return;

		report_error(NULL, "File not found: '%s', search locations are:", relative_path);
		this->get_relative_to_current_file(relative_path, absolute_path);
		report_error(NULL, "\t[ local ] %s", absolute_path);
		this->get_relative_to_main_file(relative_path, absolute_path);
		report_error(NULL, "\t[ main  ] %s", absolute_path);
		report_error_and_stop(location, "Here is the calling site");
	}

	void get_relative_to_current_file (const char* relative_path, char* absolute_path) {
		os_get_absolute_path(relative_path, absolute_path);
	}

	void get_relative_to_main_file (const char* relative_path, char* absolute_path) {
		char tmp[MAX_PATH_LENGTH];
		os_get_current_directory(tmp);
		os_set_current_directory(Compiler::inst->settings->initial_path);
		os_get_absolute_path(relative_path, absolute_path);
		os_set_current_directory(tmp);
	}
};
