#pragma once

#include "module.hpp"
#include "code_input.hpp"
#include "ast/ast.hpp"
#include "ast/parser.hpp"
#include "pipeline/pipeline.hpp"

#include <map>

struct Compiler;

struct Modules {
    std::map<const char*, Module*> cache;

	Pipeline* pipeline = new Pipeline();
    Parser* parser = new Parser();
    Lexer* lexer = new Lexer();

    Modules (Compiler* compiler);

    void handle_module_parsed (void* data);

    Module* get_module (const char* absolute_path);
    Module* load_module (const char* absolute_path);

    bool is_module_cached (const char* absolute_path);

    void print_metrics (double userInterval);
};
