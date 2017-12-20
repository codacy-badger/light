#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <deque>
#include <vector>
#include <map>

using namespace std;

struct cmp_str {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Ast_Statement_Dependency {
    vector<Ast_Ident**> symbols;
    Ast_Statement* stm = NULL;
};

struct Symbol_Resolution : Pipe {
    map<const char*, vector<Ast_Statement_Dependency*>, cmp_str> unresolved;

    void on_statement (Ast_Statement* stm);
    void on_finish ();

    void on_resolved (Ast_Statement* stm);
    bool is_unresolved (const char* name);

    void check_symbols (Ast_Statement* stm, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Expression** stm, vector<Ast_Ident**>* sym);
	void check_symbols (Ast_Function_Call** call, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Comma_Separated_Arguments** args, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Type_Definition** tydef, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Function_Type** fn_type, vector<Ast_Ident**>* sym);
};
