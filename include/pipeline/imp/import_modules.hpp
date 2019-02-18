#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

#include "pipeline/service/modules.hpp"

#include <map>
#include <vector>

struct Import_Modules : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Modules* modules = NULL;

    Import_Modules (Modules* modules) : Compiler_Pipe("Import Modules") {
        this->modules = modules;
    }

    void handle (Ast_Statement* global_statement) {
        this->ast_handle(global_statement);

        if (global_statement->stm_type == AST_STATEMENT_EXPRESSION) {
            auto exp = static_cast<Ast_Expression*>(global_statement);
            if (exp->exp_type == AST_EXPRESSION_IMPORT) {
                return;
            }
        }

        this->push_out(global_statement);
    }

    void ast_handle (Ast_Statement* stm) {
        if (stm->stm_type == AST_STATEMENT_EXPRESSION) {
            auto exp = static_cast<Ast_Expression*>(stm);
            if (exp->exp_type == AST_EXPRESSION_IMPORT) {
                auto import = static_cast<Ast_Import*>(exp);
                this->handle_import(import);
                return;
            }
        }
        Ast_Navigator::ast_handle(stm);
    }

    void ast_handle (Ast_Declaration* decl) {
        if (decl->is_constant && decl->expression) {
            if (decl->expression->exp_type == AST_EXPRESSION_IMPORT) {
                auto import = static_cast<Ast_Import*>(decl->expression);
                this->handle_import(import, decl->name);
                return;
            }
        }
        Ast_Navigator::ast_handle(decl);
    }

    void handle_import (Ast_Import* import, const char* _name = NULL) {
        this->find_file_or_error(import);

        if (import->is_include) {
            auto tmp_scope = new Ast_Scope();
            this->context->parser->parse_into(tmp_scope, import->resolved_source_file);
            auto import_location = import->scope->remove(import);
            import->scope->add(import_location, tmp_scope->statements);
            //delete tmp_scope;
            //delete import;
        } else {
            auto file_scope = this->modules->add_import(import);
            if (!_name) {
                import->scope->remove(import);
                import->scope->imports.push_back(file_scope);
            } else import->file_scope = file_scope;
        }
    }

    void ast_handle (Ast_Import* import) {
        this->context->error(import, "Invalid import statement, valid forms are:");
        this->context->error("\t- Anonymous import: \"import \"foo\"\"");
        this->context->error("\t- Named import: \"foozle :: import \"foo\"\"");
        this->context->shutdown();
    }

    void find_file_or_error (Ast_Import* import) {
        if (import->is_include) {
            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\%s.li",
                import->current_folder, import->path);
            if (!os_check_file_exists(import->resolved_source_file)) {
                this->context->error("File not found: %s\n", import->resolved_source_file);
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

            this->context->error("Module not found: %s\n", import->path);
        }
    }
};
