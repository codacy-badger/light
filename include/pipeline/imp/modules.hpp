#pragma once

#include "build_context.hpp"
#include "utils/string_vector.hpp"
#include "utils/string_map.hpp"
#include "ast/nodes.hpp"
#include "front/parser/internal_scope.hpp"

struct Modules {
    Build_Context* context = NULL;
    Ast_Scope* internal_scope = NULL;

    String_Map<Ast_Scope*> scope_cache;
    std::map<Ast_Scope*, std::vector<Ast_Scope*>> reverse_dependencies;

    void init (Build_Context* c) {
        this->internal_scope = new Internal_Scope(c->target_arch, c->target_os);
        this->context = c;
    }

    Ast_Scope* get_file_scope (const char* absolute_path) {
        if (this->scope_cache.contains(absolute_path)) {
            return this->scope_cache[absolute_path];
        } else {
            auto file_scope = new Ast_Scope();
            file_scope->location.filename = absolute_path;
    		file_scope->imports.push_back(this->internal_scope);
            this->scope_cache[absolute_path] = file_scope;
            return file_scope;
        }
    }

    Ast_Scope* add_import (Ast_Scope* from_scope, Ast_Import* import) {
        if (!this->scope_cache.contains(import->resolved_source_file)) {
            this->context->workspace->add_source_file(import->resolved_source_file);
        }

        auto file_scope_to_import = this->get_file_scope(import->resolved_source_file);
        this->reverse_dependencies[from_scope].push_back(file_scope_to_import);
        return file_scope_to_import;
    }
};
