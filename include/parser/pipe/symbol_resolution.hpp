#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <vector>
#include <map>
#include <set>

using namespace std;

struct cmp_str {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Symbol_Resolution : Pipe {
    map<const char*, vector<Ast_Statement*>, cmp_str> cache;

    bool check_symbols (Ast_Statement* stm, set<const char*, cmp_str>* sym);
    bool check_symbols (Ast_Declaration* stm, set<const char*, cmp_str>* sym);
    bool check_symbols (Ast_Block* stm, set<const char*, cmp_str>* sym);

    bool check_symbols (Ast_Expression* stm, set<const char*, cmp_str>* sym);
    bool check_symbols (Ast_Function* stm, set<const char*, cmp_str>* sym);
    bool check_symbols (Ast_Type_Instance* ty_inst, set<const char*, cmp_str>* sym);

    void on_block_begin(Ast_Block* block);
    void on_statement(Ast_Statement* stm);
    void on_block_end(Ast_Block* block);
	void on_finish ();
};
