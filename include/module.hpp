#pragma once

#include <vector>

struct Token;
struct Ast_Scope;

struct Module {
    char* absolute_path = NULL;
    const char* full_source = NULL;
    std::vector<Token*> tokens;
    Ast_Scope* global_scope = NULL;
};
