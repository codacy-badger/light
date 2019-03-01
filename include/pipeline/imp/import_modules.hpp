#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

#include "pipeline/service/modules.hpp"

struct Import_Modules : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Import_Modules () : Compiler_Pipe("Import Modules") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        this->ast_handle(global_statement);

        if (global_statement->stm_type == AST_STATEMENT_EXPRESSION) {
            auto exp = static_cast<Ast_Expression*>(global_statement);
            if (exp->exp_type == AST_EXPRESSION_IMPORT) {
                return;
            }
        }

        auto global_scope = global_statement->parent_scope;
        if (Ast_Utils::any_import_has_static_ifs(global_scope)) {
            this->requeue(global_statement);
        } else this->push_out(global_statement);
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
        if (decl->is_constant && decl->value != NULL) {
            if (decl->value->exp_type == AST_EXPRESSION_IMPORT) {
                auto import = static_cast<Ast_Import*>(decl->value);
                assert(decl->names.size > 0);
                this->handle_import(import, decl->names[0]);
                return;
            }
        }
        Ast_Navigator::ast_handle(decl);
    }

    void handle_import (Ast_Import* import, const char* _name = NULL) {
        if (import->file_scope) return;

        auto found = this->find_file_or_error(import);
        if (!found) return;

        if (import->is_include) {
            auto tmp_scope = new Ast_Scope();
            this->context->parser->parse_into(tmp_scope, import->resolved_source_file);
            auto import_location = import->scope->remove(import);
            import->scope->add(import_location, tmp_scope->statements);
            //delete tmp_scope;
            //delete import;
        } else {
            auto file_scope = this->context->modules->add_import(import);
            if (!_name) {
                import->scope->remove(import);
                import->scope->imports.push_back(file_scope);
            } else import->file_scope = file_scope;
        }
    }

    void ast_handle (Ast_Import* import) {
        this->context->error(import, "Invalid import statement, valid forms are:");
        this->context->error("\t- Anonymous import: \"import \"foo\"\"");
        this->context->error("\t- Namespaced import: \"foozle :: import \"foo\"\"");
        this->context->shutdown();
    }

    bool find_file_or_error (Ast_Import* import) {
        if (import->is_include) {
            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s.li",
                import->resolved_source_file);
            if (!os_check_file_exists(import->resolved_source_file)) {
                this->context->error("File not found: %s\n", import->resolved_source_file);
                this->context->shutdown();
                return false;
            } return true;
        } else {
            const char* searched_paths[3] = {};

            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s.li",
                import->resolved_source_file);
            if (os_check_file_exists(import->resolved_source_file)) return true;
            searched_paths[0] = _strdup(import->resolved_source_file);

            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\modules\\%s.li",
                this->context->base_path, import->path);
            if (os_check_file_exists(import->resolved_source_file)) return true;
            searched_paths[1] = _strdup(import->resolved_source_file);

            sprintf_s(import->resolved_source_file, MAX_PATH_LENGTH, "%s\\modules\\%s\\main.li",
                this->context->base_path, import->path);
            if (os_check_file_exists(import->resolved_source_file)) return true;
            searched_paths[2] = _strdup(import->resolved_source_file);

            this->context->error(import, "Module '%s' not found in search paths:", import->path);
            this->context->error("\t%s", searched_paths[0]);
            this->context->error("\t%s", searched_paths[1]);
            this->context->error("\t%s", searched_paths[2]);
            this->context->shutdown();
            return false;
        }
    }
};
