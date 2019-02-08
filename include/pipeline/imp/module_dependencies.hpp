#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Module_Dependencies : Compiler_Pipe<Ast_Scope*>, Ast_Navigator {

    Module_Dependencies () : Compiler_Pipe("Module Dependencies") {}

    void handle (Ast_Scope* file_scope) {
        Ast_Navigator::ast_handle(file_scope);
        this->push_out(file_scope);
    }

    void ast_handle (Ast_Import* import) {
        this->find_file_or_stop(import);

        this->context->workspace->add_source_file(import->resolved_source_file);
    }

    void find_file_or_stop (Ast_Import* import) {
        if (import->is_include) {
            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\%s.li",
                import->current_folder, import->path);
            printf("INCLUDE: %s\n", import->resolved_source_file);
            printf("INCLUDE: %s\n", import->resolved_source_file);

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
