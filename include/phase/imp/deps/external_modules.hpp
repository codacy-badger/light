#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"
#include "phase/pipeline/pipeline.hpp"

#include <vector>
#include <map>

struct External_Modules : Async_Phase, Ast_Navigator {
    std::map<Module*, std::vector<Ast_Directive_Import*>> module_dependencies;

    Module* current_module = NULL;
    Ast_Scope* current_scope = NULL;

    size_t foreign_functions = 0;

    External_Modules() : Async_Phase("External Modules") {
        this->bind(CE_MODULE_READY, &External_Modules::on_module_ready, this);
    }

    void handle_main_event (void* data) {
        this->current_module = reinterpret_cast<Module*>(data);

        this->ast_handle(this->current_module->global_scope);

        auto it = this->module_dependencies.find(this->current_module);
        if (it == this->module_dependencies.end()) {
            Events::trigger(this->event_to_id, this->current_module);
        }
    }

    void ast_handle (Ast_Directive_Import* import) {
        import->remove_from_scope = true;

		find_existing_absolute_path(import);

        this->module_dependencies[this->current_module].push_back(import);

        Events::trigger(CE_IMPORT_MODULE, import->absolute_path);
    }

    void ast_handle (Ast_Directive_Foreign* foreign) {
        foreign->remove_from_scope = true;

        auto new_stms = &foreign->declarations;
        auto stms = &this->current_scope->statements;
        auto foreign_stm = std::find(stms->begin(), stms->end(), foreign);
        stms->insert(foreign_stm + 1, new_stms->begin(), new_stms->end());

        this->foreign_functions += new_stms->size();
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;
        Ast_Navigator::ast_handle(scope);
        this->current_scope = tmp;
    }

    void on_module_ready (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        auto it = this->module_dependencies.begin();
        while (it != this->module_dependencies.end()) {
            auto global_scope = it->first->global_scope;
            auto imports = &it->second;

            for (int i = 0; i < imports->size();) {
                auto import = (*imports)[i];
                if (strcmp(module->absolute_path, import->absolute_path) == 0) {
                    if (import->include) {
                        global_scope->includes.push_back(module->global_scope);
                    } else global_scope->imports.push_back(module->global_scope);

                    imports->erase(imports->begin() + i);
                    break;
                } else i++;
            }

            if (imports->empty()) {
                Events::trigger(this->event_to_id, it->first);
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

	void print_extra_metrics() {
		print_extra_metric("Foreign functions", "%zd", this->foreign_functions);
	}
};
