#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

#include "compiler.hpp"

struct External_Modules : Scoped_Statement_Pipe {
	External_Modules () { this->pipe_name = "External_Modules"; }

    void handle (Ast_Directive_Import** import_ptr) {
		auto import = (*import_ptr);

		this->delete_current_statement();
		find_existing_absolute_path(import);

		auto module = this->get_external_module(import->absolute_path);
		this->current_scope->external_modules.push_back(module);
    }

	void find_existing_absolute_path (Ast_Directive_Import* import) {
		// @TODO this method should use the path in the location ptr to
		// compute the absolute path relative to the current folder
		this->get_relative_to_current_file(import);
		if (os_check_file_exists(import->absolute_path)) return;

		this->get_relative_to_main_file(import);
		if (os_check_file_exists(import->absolute_path)) return;

		report_error(NULL, "File not found: '%s', search locations are:", import->path);
		this->get_relative_to_current_file(import);
		report_error(NULL, "\t%s", import->absolute_path);
		this->get_relative_to_main_file(import);
		report_error(NULL, "\t%s", import->absolute_path);
		report_error_and_stop(&import->location, "Here is the calling site");
	}

	void get_relative_to_current_file (Ast_Directive_Import* import) {
		os_get_absolute_path_relative_to(import->path, import->location.filename, import->absolute_path);
	}

	void get_relative_to_main_file (Ast_Directive_Import* import) {
		char tmp[MAX_PATH_LENGTH];
		os_get_current_directory(tmp);
		os_set_current_directory(Compiler::inst->settings->initial_path);
		os_get_absolute_path(import->path, import->absolute_path);
		os_set_current_directory(tmp);
	}

	Module* get_external_module (char* absolute_path) {
		this->total_time += os_time_user_stop(this->start_time);
		auto module = Compiler::inst->modules->get_module(absolute_path);
		this->start_time = os_get_user_time();
		return module;
	}
};
