#pragma once

#include <string.h>
#include <vector>

#include "parser/ast.hpp"

using namespace std;

struct Bytecode_Types {
    vector<Ast_Type_Definition*> all_types;

    bool contains (Ast_Type_Definition* type_def);
    void add (Ast_Type_Definition* type_def);
    void print (Ast_Type_Definition* type_def, FILE* out = NULL);

    Ast_Type_Definition* get_struct_type (const char* name);
    Ast_Type_Definition* get_pointer_type (Ast_Type_Definition* base);
    Ast_Type_Definition* get_function_type (vector<Ast_Declaration*> parameter_decls, Ast_Expression* return_type);
};
