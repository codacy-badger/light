#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"
#include "utils/string_map.hpp"

#include <map>
#include <vector>

struct Import_Modules : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Modules* modules = NULL;

    Import_Modules (Modules* modules) : Compiler_Pipe("Import Modules") {
        this->modules = modules;
    }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Declaration* decl) {
        if (decl->is_constant && decl->expression) {
            if (decl->expression->exp_type == AST_EXPRESSION_IMPORT) {
                auto import = static_cast<Ast_Import*>(decl->expression);
                this->find_file_or_error(import);

                auto file_scope = this->modules->add_import(import);
                decl->scope->named_imports[decl->name] = file_scope;
                import->file_scope = file_scope;

                return;
            }
        }
        Ast_Navigator::ast_handle(decl);
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
