#pragma once

#include "code_input.hpp"
#include "ast/ast.hpp"
#include "ast/parser.hpp"
#include "pipeline/pipeline.hpp"

#include <map>

struct Compiler;

struct Modules {
    std::map<const char*, Ast_Scope*> cache;

	Ast_Scope* internal_scope = new Ast_Scope();
	Pipeline* pipeline = new Pipeline();
    Parser* parser = new Parser();
    Lexer* lexer = new Lexer();

    Modules (Compiler* compiler);

    Ast_Scope* get_module (const char* absolute_path);
    Ast_Scope* load_module (const char* absolute_path);

    bool is_module_cached (const char* absolute_path);

    void print_metrics (double userInterval);
};
