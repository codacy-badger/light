#include "ast/types.hpp"

#include "ast/nodes.hpp"

Ast_Struct_Type* build_type_string () {
    auto type_string = new Ast_Struct_Type("string");
    type_string->inferred_type = Types::type_type;

    auto length_decl = new Ast_Declaration();
    length_decl->type = Types::type_u64;
    length_decl->name = "length";

    auto data_decl = new Ast_Declaration();
    data_decl->type = new Ast_Pointer_Type(Types::type_byte);
    data_decl->name = "data";

    type_string->scope.add(length_decl);
    type_string->scope.add(data_decl);

    return type_string;
}

Ast_Struct_Type* build_type_any () {
    auto type_any = new Ast_Struct_Type("any");
    type_any->inferred_type = Types::type_type;

    auto type_decl = new Ast_Declaration();
    type_decl->type = Types::type_u8;
    type_decl->name = "type";

    auto value_decl = new Ast_Declaration();
    value_decl->type = new Ast_Pointer_Type(Types::type_void);
    value_decl->name = "value";

    type_any->scope.add(type_decl);
    type_any->scope.add(value_decl);

    return type_any;
}

Ast_Struct_Type* Types::type_namespace  = new Ast_Struct_Type("namespace",  0);
Ast_Struct_Type* Types::type_type       = new Ast_Struct_Type("type",       0, true /* primitive */ );
Ast_Struct_Type* Types::type_void       = new Ast_Struct_Type("void",       0, true);
Ast_Struct_Type* Types::type_bool       = new Ast_Struct_Type("bool",       1, true);
Ast_Struct_Type* Types::type_byte       = new Ast_Struct_Type("byte",       1, true);
Ast_Struct_Type* Types::type_s8         = new Ast_Struct_Type("s8",         1, true, true /* number */ , true /* signed */ );
Ast_Struct_Type* Types::type_s16        = new Ast_Struct_Type("s16",        2, true, true, true);
Ast_Struct_Type* Types::type_s32        = new Ast_Struct_Type("s32",        4, true, true, true);
Ast_Struct_Type* Types::type_s64        = new Ast_Struct_Type("s64",        8, true, true, true);
Ast_Struct_Type* Types::type_u8         = new Ast_Struct_Type("u8",         1, true, true);
Ast_Struct_Type* Types::type_u16        = new Ast_Struct_Type("u16",        2, true, true);
Ast_Struct_Type* Types::type_u32        = new Ast_Struct_Type("u32",        4, true, true);
Ast_Struct_Type* Types::type_u64        = new Ast_Struct_Type("u64",        8, true, true);
Ast_Struct_Type* Types::type_f32        = new Ast_Struct_Type("f32",        4, true, true);
Ast_Struct_Type* Types::type_f64        = new Ast_Struct_Type("f64",        8, true, true);
Ast_Struct_Type* Types::type_usize      = new Ast_Struct_Type("usize",      8, true, true);
Ast_Struct_Type* Types::type_string     = build_type_string();
Ast_Struct_Type* Types::type_any        = build_type_any();
