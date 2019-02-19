#pragma once

struct Ast_Struct_Type;

struct Types {
    static Ast_Struct_Type* type_namespace;
	static Ast_Struct_Type* type_type;
	static Ast_Struct_Type* type_void;
	static Ast_Struct_Type* type_bool;
	static Ast_Struct_Type* type_byte;
	static Ast_Struct_Type* type_s8;
	static Ast_Struct_Type* type_s16;
	static Ast_Struct_Type* type_s32;
	static Ast_Struct_Type* type_s64;
	static Ast_Struct_Type* type_u8;
	static Ast_Struct_Type* type_u16;
	static Ast_Struct_Type* type_u32;
	static Ast_Struct_Type* type_u64;
	static Ast_Struct_Type* type_usize;
	static Ast_Struct_Type* type_f32;
	static Ast_Struct_Type* type_f64;
	static Ast_Struct_Type* type_string;
    static Ast_Struct_Type* type_any;
};
