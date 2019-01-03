#pragma once

#include <vector>

struct Token;
struct Ast_Scope;

struct Module {
    char* absolute_path = NULL;
    std::vector<Token*>* tokens = NULL;
    Ast_Scope* global_scope = NULL;
};
