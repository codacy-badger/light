#pragma once

#include <string.h>
#include <vector>
#include <map>

#include "parser/ast.hpp"

using namespace std;

struct Types {
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Expression*, Ast_Slice_Type*> sli_types;

	Ast_Pointer_Type* get_pointer_type (Ast_Expression* base_type);

	Ast_Slice_Type* get_slice_type (Ast_Expression* base_type);

    Ast_Function_Type* get_function_type (Ast_Function* func);

    bool is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to);
};
