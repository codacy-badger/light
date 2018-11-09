#pragma once

#include "pipe.hpp"
#include "parser/parser.hpp"

#include <vector>

using namespace std;

struct Pipeline {
    Parser* parser = new Parser();
    vector<Pipe*> pipes;

    Ast_Block* global_scope = NULL;

	deque<Ast_Import*> pending_imports;
	vector<char*> imported_files;

    Pipeline ();

    void run(const char* filepath);
    void handle_file(const char* filepath);
    void handle_stm(Ast_Statement* stm, int from_index = 0);

    void print_compiler_metrics (double total_time);
};
