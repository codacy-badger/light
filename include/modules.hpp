#pragma once

#include "module.hpp"
#include "ast/ast.hpp"
#include "ast/parser.hpp"
#include "pipeline/pipeline.hpp"

#include <map>

struct Compiler;

struct Modules {
    std::map<const char*, Module*> cache;

	Pipeline* pipeline = new Pipeline();

    Modules (Compiler* compiler);

    void handle_module_parsed (void* data);
    void on_import_module (void* data);
    void on_module_ready (void* data);

    bool is_module_cached (const char* absolute_path);

    void print_metrics (double userInterval);
};
