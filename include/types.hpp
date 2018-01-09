#pragma once

#include <string.h>
#include <vector>
#include <map>

#include "parser/ast.hpp"

using namespace std;

struct cmp_str_types {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Types {
    map<const char*, Ast_Struct_Type*, cmp_str_types> struct_types;
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Expression*, map<uint64_t, Ast_Array_Type*>> arr_types;
    map<Ast_Expression*, Ast_Slice_Type*> sli_types;
    vector<Ast_Function_Type*> func_types;

	Ast_Struct_Type* get_struct_type (const char* name);

	Ast_Pointer_Type* get_pointer_type (Ast_Type_Instance* base_type);
	Ast_Pointer_Type* get_or_create_pointer_type (Ast_Expression* base_type);

    Ast_Array_Type* get_unique_array_type (Ast_Array_Type* arr_type);
	Ast_Array_Type* get_or_create_array_type (Ast_Expression* base_type, uint64_t size);

	Ast_Slice_Type* get_unique_slice_type (Ast_Slice_Type* arr_type);
	Ast_Slice_Type* get_or_create_slice_type (Ast_Expression* base_type);

    Ast_Function_Type* get_unique_function_type (Ast_Function_Type* func_type);
    Ast_Function_Type* get_or_create_function_type (Ast_Function* func);

    bool is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to);

    void compute_type_name_if_needed (Ast_Type_Instance* type_def);
};
