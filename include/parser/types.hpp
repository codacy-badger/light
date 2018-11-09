#pragma once

#include <vector>
#include <map>

#include "parser/ast.hpp"

using namespace std;

struct Types {
    vector<Ast_Type_Instance*> all_types;

    map<Ast_Type_Instance*, Ast_Pointer_Type*> ptr_types;
    map<Ast_Type_Instance*, Ast_Slice_Type*> sli_types;
    map<Ast_Expression*, map<uint64_t, Ast_Array_Type*>> arr_types;
    vector<Ast_Function_Type*> func_types;

    Ast_Expression* value_false = NULL;
    Ast_Expression* value_true = NULL;

	static Ast_Struct_Type* type_def_type;
	static Ast_Struct_Type* type_def_void;
	static Ast_Struct_Type* type_def_bool;
	static Ast_Struct_Type* type_def_s8;
	static Ast_Struct_Type* type_def_s16;
	static Ast_Struct_Type* type_def_s32;
	static Ast_Struct_Type* type_def_s64;
	static Ast_Struct_Type* type_def_u8;
	static Ast_Struct_Type* type_def_u16;
	static Ast_Struct_Type* type_def_u32;
	static Ast_Struct_Type* type_def_u64;
	static Ast_Struct_Type* type_def_f32;
	static Ast_Struct_Type* type_def_f64;

    Types () {
        auto lit = new Ast_Literal();
        lit->inferred_type = this->type_def_bool;
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = 0;
        this->value_false = lit;

        lit = new Ast_Literal();
        lit->inferred_type = this->type_def_bool;
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = 1;
        this->value_true = lit;
    }

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
