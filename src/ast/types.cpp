#include "ast/types.hpp"

Ast_Struct_Type* Types::type_type = new Ast_Struct_Type("type");
Ast_Struct_Type* Types::type_void = new Ast_Struct_Type("void");
Ast_Struct_Type* Types::type_bool = new Ast_Struct_Type("bool", 1, true /* primitive */ );
Ast_Struct_Type* Types::type_s8   = new Ast_Struct_Type("s8", 	1, true, true /* signed */ );
Ast_Struct_Type* Types::type_s16  = new Ast_Struct_Type("s16",  2, true, true);
Ast_Struct_Type* Types::type_s32  = new Ast_Struct_Type("s32",  4, true, true);
Ast_Struct_Type* Types::type_s64  = new Ast_Struct_Type("s64",  8, true, true);
Ast_Struct_Type* Types::type_u8   = new Ast_Struct_Type("u8",   1, true);
Ast_Struct_Type* Types::type_u16  = new Ast_Struct_Type("u16",  2, true);
Ast_Struct_Type* Types::type_u32  = new Ast_Struct_Type("u32",  4, true);
Ast_Struct_Type* Types::type_u64  = new Ast_Struct_Type("u64",  8, true);
Ast_Struct_Type* Types::type_f32  = new Ast_Struct_Type("f32",  4, true);
Ast_Struct_Type* Types::type_f64  = new Ast_Struct_Type("f64",  8, true);
Ast_Slice_Type* Types::type_string  = new Ast_Slice_Type(Types::type_u8, "string");
Ast_Struct_Type* Types::type_any  = build_type_any();
