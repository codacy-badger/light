#pragma once

#include "ast/ast.hpp"
#include "pipeline/pipeline.hpp"

#include <map>

struct Modules {
    std::map<char*, Ast_Scope*> cache;

    Pipeline* pipeline;

    Modules (Pipeline* pipeline) {
        this->pipeline = pipeline;
        ASSERT(this->pipeline != NULL);
    }

    Ast_Scope* get_module (char* name) {
        if (!this->is_module_cached(name)) {
            auto load_success = this->load_module(name);
            if (load_success) return this->cache[name];
        }
        return NULL;
    }

    bool is_module_cached (char* name) {
        auto it = this->cache.find(name);
        return it != this->cache.end();
    }

    bool load_module (char* name) {
        report_debug(NULL, "TODO: load source file: \n\t%s", name);
        return false;
    }

    bool free_module (char* name) {
        if (this->is_module_cached(name)) {
            this->cache.erase(name);
        }
    }
};
