#pragma once

#include "build_context.hpp"
#include "utils/string_vector.hpp"
#include "utils/string_map.hpp"
#include "ast/nodes.hpp"

struct Modules {
    Build_Context* context = NULL;

    String_Vector imported_scopes;
    String_Map<Ast_Scope*> scope_cache;
    std::map<Ast_Scope*, std::vector<Ast_Scope*>> imports;

    void init (Build_Context* c) { this->context = c; }

    void add_import (Ast_Import* import) {
        printf("Adding IMPORT for %s: '%s'\n", import->get_absolute_path(), import->resolved_source_file);

        if (this->imported_scopes.contains(import->resolved_source_file)) {
            printf("We've already imported this module: %s\n", import->resolved_source_file);
            return;
        }

        if (!this->scope_cache.contains(import->resolved_source_file)) {
            this->imported_scopes.push_back(import->resolved_source_file);
            this->context->workspace->add_source_file(import->resolved_source_file);
        } else printf("Module already cached: %s\n", import->resolved_source_file);
    }
};
