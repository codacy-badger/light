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
    vector<Ast_Ident**> unresolved_symbols;
    Ast_Statement* stm = NULL;
};

struct Symbol_Resolution : Pipe {
    map<const char*, vector<Ast_Statement_Dependency*>, cmp_str> unresolved_symbols;

    void on_statement (Ast_Statement* stm);
    void on_finish ();

    void on_resolved (Ast_Statement* stm);
    bool is_unresolved (const char* name);

    void check_symbols (Ast_Statement* stm, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Declaration* stm, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_If* _if, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_While* _while, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Block* stm, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Return* ret, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Expression** stm, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Ident** ident, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Binary** binary, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Unary** unary, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Function** stm, vector<Ast_Ident**>* sym);
	void check_symbols (Ast_Function_Call** call, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Comma_Separated_Arguments** args, vector<Ast_Ident**>* sym);

    void check_symbols (Ast_Type_Definition** tydef, vector<Ast_Ident**>* sym);
	void check_symbols (Ast_Struct_Type** _struct, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Pointer_Type** ptr_type, vector<Ast_Ident**>* sym);
    void check_symbols (Ast_Function_Type** fn_type, vector<Ast_Ident**>* sym);
};
