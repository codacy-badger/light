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

	Ast_Struct_Type* type_def_type = new Ast_Struct_Type("type",   0);;
	Ast_Struct_Type* type_def_void = new Ast_Struct_Type("void",   0);;
	Ast_Struct_Type* type_def_bool = new Ast_Struct_Type("bool",   1, true /* primitive */ );;
	Ast_Struct_Type* type_def_s8   = new Ast_Struct_Type("s8", 	   1, true, true /* signed */ );;
	Ast_Struct_Type* type_def_s16  = new Ast_Struct_Type("s16",    2, true, true);;
	Ast_Struct_Type* type_def_s32  = new Ast_Struct_Type("s32",    4, true, true);;
	Ast_Struct_Type* type_def_s64  = new Ast_Struct_Type("s64",    8, true, true);;
	Ast_Struct_Type* type_def_u8   = new Ast_Struct_Type("u8",     1, true);;
	Ast_Struct_Type* type_def_u16  = new Ast_Struct_Type("u16",    2, true);;
	Ast_Struct_Type* type_def_u32  = new Ast_Struct_Type("u32",    4, true);;
	Ast_Struct_Type* type_def_u64  = new Ast_Struct_Type("u64",    8, true);;
	Ast_Struct_Type* type_def_f32  = new Ast_Struct_Type("f32",    4, true);;
	Ast_Struct_Type* type_def_f64  = new Ast_Struct_Type("f64",    8, true);;

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
