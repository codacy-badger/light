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

struct Cast_Instance {
    Ast_Function* function = NULL;
    bool is_implicid = false;
};

struct Types {
    map<const char*, Ast_Struct_Type*, cmp_str_types> struct_types;
    map<Ast_Expression*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Expression*, Ast_Array_Type*> arr_types;
    vector<Ast_Function_Type*> func_types;

    map<Ast_Type_Definition*, map<Ast_Type_Definition*, Cast_Instance*>> casts;

    Ast_Type_Definition* get_unique_type (Ast_Type_Definition* type_def);

	Ast_Struct_Type* get_struct_type (const char* name);
    Ast_Struct_Type* get_unique_struct_type (Ast_Struct_Type* _struct);

	Ast_Pointer_Type* get_pointer_type (Ast_Type_Definition* base_type);
	Ast_Pointer_Type* get_or_create_pointer_type (Ast_Expression* base_type);
    Ast_Pointer_Type* get_unique_pointer_type (Ast_Pointer_Type* ptr_type);

    Ast_Array_Type* get_unique_array_type (Ast_Array_Type* arr_type);

    Ast_Function_Type* get_unique_function_type (Ast_Function_Type* func_type);
    Ast_Function_Type* get_or_create_function_type (Ast_Function* func);

    void add_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to, Ast_Function* func, bool is_implicid = false);
    Cast_Instance* get_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to);

    bool is_implicid_cast (Ast_Type_Definition* type_from, Ast_Type_Definition* type_to);

    void compute_type_name_if_needed (Ast_Type_Definition* type_def);
};
