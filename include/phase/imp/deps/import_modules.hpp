#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"

#include <vector>
#include <map>

struct Import_Modules : Phase, Ast_Navigator {
    std::map<Ast_Scope*, std::vector<Ast_Import*>> dependencies;
    const char* initial_path;

    size_t foreign_functions = 0;

    Import_Modules() : Phase("Import Modules", CE_MODULE_RESOLVE_IMPORTS, true) {
        this->bind(CE_MODULE_READY);
    }

    void on_start (Compiler_Settings* settings) {
        this->initial_path = settings->initial_path;
    }

    void on_event (Event event) {
        switch (event.id) {
            case CE_MODULE_RESOLVE_IMPORTS: {
                auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

                Ast_Navigator::ast_handle(global_scope);

                auto it = this->dependencies.find(global_scope);
                if (it == this->dependencies.end()) {
                    this->push(global_scope);
                }
                break;
            }
            case CE_MODULE_READY: {
                this->on_module_ready(event.data);
                break;
            }
        }
    }

    void ast_handle (Ast_Import* import) {
        import->remove_from_scope = true;

		find_existing_absolute_path(import);

        if (this->current_scope->is_global()) {
            this->dependencies[this->current_scope].push_back(import);
        }

        Events::trigger(CE_IMPORT_MODULE, import->absolute_path);
    }

    void ast_handle (Ast_Foreign* foreign) {
        foreign->remove_from_scope = true;

        auto new_stms = &foreign->declarations;
        auto stms = &this->current_scope->statements;
        auto foreign_stm = std::find(stms->begin(), stms->end(), foreign);
        stms->insert(foreign_stm + 1, new_stms->begin(), new_stms->end());

        this->foreign_functions += new_stms->size();
    }

    void on_module_ready (void* data) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(data);

        auto it = this->dependencies.begin();
        while (it != this->dependencies.end()) {
            auto dep_global_scope = it->first;
            auto imports = &it->second;

            for (int i = 0; i < imports->size();) {
                auto import = (*imports)[i];
                auto scope_absolute_path = global_scope->get_absolute_path();
                if (strcmp(scope_absolute_path, import->absolute_path) == 0) {
                    if (import->include) {
                        dep_global_scope->includes.push_back(global_scope);
                    } else dep_global_scope->imports.push_back(global_scope);

                    imports->erase(imports->begin() + i);
                    break;
                } else i++;
            }

            if (imports->empty()) {
                Events::trigger(this->event_to_id, it->first);
                it = this->dependencies.erase(it);
            } else it++;
        }
    }

	void find_existing_absolute_path (Ast_Import* import) {
		// @TODO this method should use the path in the location ptr to
		// compute the absolute path relative to the current folder
		this->get_relative_to_current_file(import);
		if (os_check_file_exists(import->absolute_path)) return;

		this->get_relative_to_main_file(import);
		if (os_check_file_exists(import->absolute_path)) return;

		Logger::error("File not found: '%s', search locations are:", import->path);
		this->get_relative_to_current_file(import);
		Logger::error("\t%s", import->absolute_path);
		this->get_relative_to_main_file(import);
		Logger::error("\t%s", import->absolute_path);
		Logger::error(import, "Here is the calling site");
	}

	void get_relative_to_current_file (Ast_Import* import) {
		os_get_absolute_path_relative_to(import->path, import->location.filename, import->absolute_path);
	}

	void get_relative_to_main_file (Ast_Import* import) {
		char tmp[MAX_PATH_LENGTH];
		os_get_current_directory(tmp);
		os_set_current_directory(this->initial_path);
		os_get_absolute_path(import->path, import->absolute_path);
		os_set_current_directory(tmp);
	}

	void print_extra_metrics() {
		print_extra_metric("Foreign functions", "%zd", this->foreign_functions);
	}
};
