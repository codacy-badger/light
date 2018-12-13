#pragma once

#include "ast/ast.hpp"
#include "ast/parser.hpp"
#include "pipeline/pipeline.hpp"

#include <map>

struct Modules {
    std::map<char*, Ast_Scope*> cache;

	Ast_Scope* internal_scope = Ast_Factory::create_node<Ast_Scope>();
	Pipeline* pipeline = new Pipeline();
    Parser* parser = new Parser();

    Modules ();

    Ast_Scope* get_module (char* absolute_path);
    Ast_Scope* load_module (char* absolute_path);
    bool is_module_cached (char* absolute_path);
    bool free_module (char* absolute_path);
};
