#pragma once

#include "pipe.hpp"
#include "ast/parser.hpp"

#include <vector>

using namespace std;

struct Pipeline {
    vector<Pipe*> pipes;

    Pipeline* pipe (Pipe* pipe);

    void process (Ast_Scope* scope, size_t start_index = 0);

    void print_metrics (double total_time);
};
