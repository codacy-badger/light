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
    set<const char*, cmp_str> unresolved_symbols;
    Ast_Statement* stm = NULL;
};

struct Symbol_Resolution : Pipe {
    map<const char*, set<Ast_Statement_Dependency*>, cmp_str> unresolved_symbols;

    map<const char*, set<Ast_Type_Definition**>, cmp_str> unresolved_type_defn_references;
    map<const char*, set<Ast_Declaration**>, cmp_str> unresolved_decl_references;

    void on_statement (Ast_Statement* stm);
    void on_finish ();

    bool is_unresolved (const char* name);

    void check_symbols (Ast_Statement* stm, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Declaration* stm, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Block* stm, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Return* ret, set<const char*, cmp_str>* sym);

    void check_symbols (Ast_Expression* stm, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Binary* binary, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Unary* unary, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Function* stm, set<const char*, cmp_str>* sym);

    void check_symbols (Ast_Type_Instance* ty_inst, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Pointer_Type* ptr_type, set<const char*, cmp_str>* sym);
    void check_symbols (Ast_Function_Type* fn_type, set<const char*, cmp_str>* sym);
};
