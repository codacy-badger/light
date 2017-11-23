#pragma once

#include "parser/pipes.hpp"

#include <map>
#include <vector>

using namespace std;

struct Unique_Types : Pipe {
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    vector<Ast_Function_Type*> func_types;

    void on_statement (Ast_Statement* stm);

    void unique (Ast_Statement* stm);
    void unique (Ast_Expression** exp);
    void unique (Ast_Type_Definition** type_def);
    void unique (Ast_Pointer_Type** ptr_type);
    void unique (Ast_Function_Type** func_type);
};
