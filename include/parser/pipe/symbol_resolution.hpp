#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <deque>
#include <vector>
#include <map>

using namespace std;

struct Symbol_Resolution : Pipe {
    map<Ast_Statement*, vector<Ast_Ident**>> unresolved;

    void on_statement (Ast_Statement* stm);
    void on_finish ();

    void on_resolved (Ast_Statement* stm);
    bool is_unresolved (const char* name);

    void check_symbols (Ast_Statement* stm, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Expression** stm, vector<Ast_Ident**>* sym);
	void check_symbols (Ast_Function_Call** call, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Type_Definition** tydef, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Function_Type** fn_type, vector<Ast_Ident**>* sym);
};
