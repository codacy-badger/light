#pragma once

#include <vector>
#include <map>

#include "ast/ast.hpp"

using namespace std;

enum Internal_Type : uint8_t {
    INTERNAL_TYPE_UNDEFINED  = 0,
    INTERNAL_TYPE_VOID       = 1,
    INTERNAL_TYPE_BOOL       = 2,
    INTERNAL_TYPE_INTEGER    = 3,
    INTERNAL_TYPE_DECIMAL    = 4,
    INTERNAL_TYPE_STRING     = 5,
    INTERNAL_TYPE_ANY        = 6,
};

struct Types {
    vector<Ast_Type_Instance*> all_types;

    map<Ast_Type_Instance*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Type_Instance*, Ast_Slice_Type*> sli_types;
    map<Ast_Expression*, map<uint64_t, Ast_Array_Type*>> arr_types;
    vector<Ast_Function_Type*> func_types;

    // TODO: this should be in some sort of ConstantValues file
    static Ast_Expression* value_false;
    static Ast_Expression* value_true;

	static Ast_Struct_Type* type_type;
	static Ast_Struct_Type* type_void;
	static Ast_Struct_Type* type_bool;
	static Ast_Struct_Type* type_s8;
	static Ast_Struct_Type* type_s16;
	static Ast_Struct_Type* type_s32;
	static Ast_Struct_Type* type_s64;
	static Ast_Struct_Type* type_u8;
	static Ast_Struct_Type* type_u16;
	static Ast_Struct_Type* type_u32;
	static Ast_Struct_Type* type_u64;
	static Ast_Struct_Type* type_f32;
	static Ast_Struct_Type* type_f64;
	static Ast_Slice_Type* type_string;
    static Ast_Struct_Type* type_any;

    static Internal_Type get_internal_type (Ast_Type_Instance* type);

    void add_type_if_new (Ast_Type_Instance* type);

    void add_struct_type_if_new (Ast_Struct_Type* _struct);

    void add_pointer_type_if_new (Ast_Pointer_Type* ptr_type);

    void add_array_type_if_new (Ast_Array_Type* arr_type);

    void add_function_type_if_new (Ast_Function_Type*);

    void add_new_global_unique_type (Ast_Type_Instance* type_inst);

	Ast_Pointer_Type* get_pointer_type (Ast_Expression* base_type);

	Ast_Slice_Type* get_slice_type (Ast_Expression* base_type);

    bool is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to);
};
