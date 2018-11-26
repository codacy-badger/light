#pragma once

#include "pipe.hpp"
#include "ast/parser.hpp"

#include <vector>

using namespace std;

struct Pipeline {
    Parser* parser = new Parser();
    vector<Pipe*> pipes;

    Ast_Block* global_scope = NULL;

	deque<Ast_Import*> pending_imports;
	vector<const char*> imported_files;

    Pipeline ();

    void add_pipe(Pipe* pipe);

    void run(const char* filepath);
    void handle_file(const char* filepath);
    void handle_stm(Ast_Statement* stm, int from_index = 0);
    void handle_import(Ast_Import* import);

    void print_compiler_metrics (double total_time);
};
