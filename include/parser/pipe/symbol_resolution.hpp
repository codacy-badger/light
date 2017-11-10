#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <deque>
#include <set>
#include <map>

using namespace std;

struct cmp_str {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Ast_Statement_Dependency {
    set<Ast_Ident**> unresolved_symbols;
    Ast_Statement* stm = NULL;
};

struct Symbol_Resolution : Pipe {
    map<const char*, set<Ast_Statement_Dependency*>, cmp_str> unresolved_symbols;

    void on_statement (Ast_Statement* stm);
    void on_finish ();

    void on_resolved (Ast_Statement* stm);
    bool is_unresolved (const char* name);

    void check_symbols (Ast_Statement* stm, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Declaration* stm, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Block* stm, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Return* ret, set<Ast_Ident**>* sym);

    void check_symbols (Ast_Expression** stm, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Ident** ident, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Binary** binary, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Unary** unary, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Function** stm, set<Ast_Ident**>* sym);

    void check_symbols (Ast_Type_Definition** tydef, set<Ast_Ident**>* sym);
	void check_symbols (Ast_Struct_Type** _struct, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Pointer_Type** ptr_type, set<Ast_Ident**>* sym);
    void check_symbols (Ast_Function_Type** fn_type, set<Ast_Ident**>* sym);
};
