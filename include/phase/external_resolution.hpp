#pragma once

#include "async_phase.hpp"
#include "ast/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

#include <vector>
#include <map>

struct External_Resolution : Async_Phase, Ast_Navigator {
    std::map<Module*, std::vector<const char*>> module_dependencies;

    External_Resolution() : Async_Phase("External Resolution", CE_MODULE_PARSED) {
        Events::add_observer(CE_MODULE_READY, &External_Resolution::on_module_ready, this);
    }

    void handle (void* data) {
        auto module = reinterpret_cast<Module*>(data);
        Ast_Navigator::ast_handle(module->global_scope);
    }

    void ast_handle (Ast_Directive_Import* import) {
		find_existing_absolute_path(import);
        Events::trigger(CE_IMPORT_MODULE, import->absolute_path);
    }

    void ast_handle (Ast_Directive_Foreign*) {
        //printf("Handling #foreign directive!\n");
    }

    void on_module_ready (void*) {
        //printf("Handling module ready!\n");
        //auto module = reinterpret_cast<Module*>(data);

        /*for (auto entry : this->module_dependencies) {

        }*/
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
		os_set_current_directory(this->settings->initial_path);
		os_get_absolute_path(import->path, import->absolute_path);
		os_set_current_directory(tmp);
	}
};
