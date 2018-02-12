#pragma once

#include <string.h>
#include <vector>
#include <map>

#include "parser/ast.hpp"

using namespace std;

struct Types {
    vector<Ast_Type_Instance*> all_types;

    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Expression*, Ast_Slice_Type*> sli_types;
    map<Ast_Expression*, map<uint64_t, Ast_Array_Type*>> arr_types;
    vector<Ast_Function_Type*> func_types;

    void add_type_if_new (Ast_Type_Instance* type);

    void add_struct_type_if_new (Ast_Struct_Type* _struct);

    void add_pointer_type_if_new (Ast_Pointer_Type* ptr_type);

    void add_array_type_if_new (Ast_Array_Type* arr_type);

    void add_function_type_if_new (Ast_Function_Type*);

    void add_new_global_unique_type (Ast_Type_Instance* type_inst);

	Ast_Pointer_Type* get_pointer_type (Ast_Expression* base_type);

	Ast_Slice_Type* get_slice_type (Ast_Expression* base_type);

    Ast_Function_Type* build_function_type (Ast_Function* func);

    bool is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to);
};
