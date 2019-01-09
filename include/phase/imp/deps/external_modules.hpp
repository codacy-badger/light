#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"
#include "phase/pipeline/pipeline.hpp"

#include <vector>
#include <map>

struct External_Modules : Async_Phase, Ast_Navigator {
    std::map<Module*, std::vector<const char*>> module_dependencies;

    Module* current_module = NULL;

    External_Modules() : Async_Phase("External Modules", CE_MODULE_RESOLVE_IMPORTS) {
        Events::add_observer(CE_MODULE_READY, &External_Modules::on_module_ready, this);
    }

    void on_event (void* data) {
        this->current_module = reinterpret_cast<Module*>(data);

        Ast_Navigator::ast_handle(this->current_module->global_scope);

        auto it = this->module_dependencies.find(this->current_module);
        if (it == this->module_dependencies.end()) {
            Events::trigger(CE_MODULE_RESOLVE_SYMBOLS, this->current_module);
        }
    }

    void ast_handle (Ast_Directive_Import* import) {
        import->remove_from_scope = true;
        
		find_existing_absolute_path(import);

        this->module_dependencies[this->current_module].push_back(import->absolute_path);

        Events::trigger(CE_IMPORT_MODULE, import->absolute_path);
    }

    void on_module_ready (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        auto it = this->module_dependencies.begin();
        while (it != this->module_dependencies.end()) {
            auto modules = &it->second;

            auto _it = std::find(modules->begin(), modules->begin(), module->absolute_path);
            if (_it != modules->end()) modules->erase(_it);

            if (modules->empty()) {
                Events::trigger(CE_MODULE_RESOLVE_SYMBOLS, it->first);
                it = this->module_dependencies.erase(it);
            } else it++;
        }
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
