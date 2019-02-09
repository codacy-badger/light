#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"
#include "utils/string_map.hpp"

#include <map>
#include <vector>

struct Module_Dependencies : Compiler_Pipe<Ast_Scope*>, Ast_Navigator {
    Modules* modules = NULL;

    Module_Dependencies (Modules* modules) : Compiler_Pipe("Module Dependencies") {
        this->modules = modules;
    }

    void handle (Ast_Scope* file_scope) {
        Ast_Navigator::ast_handle(file_scope);
        this->push_out(file_scope);
    }

    void ast_handle (Ast_Import* import) {
        this->find_file_or_error(import);

        if (import->is_include) {
            auto tmp_scope = new Ast_Scope();
            this->context->parser->parse_into(tmp_scope, import->resolved_source_file);
            auto import_location = import->scope->remove(import);
            import->scope->add(import_location, tmp_scope->statements);
            delete tmp_scope;
            delete import;
        } else {
            import->scope->remove(import);
            auto file_scope = this->modules->add_import(import);
            import->scope->imports.push_back(file_scope);
        }
    }

    void find_file_or_error (Ast_Import* import) {
        if (import->is_include) {
            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\%s.li",
                import->current_folder, import->path);
            if (!os_check_file_exists(import->resolved_source_file)) {
                this->print_error("File not found: %s\n", import->resolved_source_file);
            }
        } else {
            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\%s.li",
                import->current_folder, import->path);
            if (os_check_file_exists(import->resolved_source_file)) return;

            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\modules\\%s.li",
                this->context->base_path, import->path);
            if (os_check_file_exists(import->resolved_source_file)) return;

            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\modules\\%s\\main.li",
                this->context->base_path, import->path);
            if (os_check_file_exists(import->resolved_source_file)) return;

            this->print_error("Module not found: %s\n", import->path);
        }
    }
};
