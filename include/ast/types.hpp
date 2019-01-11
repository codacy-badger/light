#pragma once

#include <vector>
#include <map>

#include "ast/ast.hpp"

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
	static Ast_Struct_Type* type_usize;
	static Ast_Struct_Type* type_f32;
	static Ast_Struct_Type* type_f64;
	static Ast_Slice_Type* type_string;
    static Ast_Struct_Type* type_any;

    static Internal_Type get_internal_type (Ast_Type_Instance* type) {
    	if (type == Types::type_void) 	return INTERNAL_TYPE_VOID;
    	if (type == Types::type_bool) 	return INTERNAL_TYPE_BOOL;
    	if (type == Types::type_s8) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_s16) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_s32) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_s64) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_u8) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_u16) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_u32) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_u64) 	return INTERNAL_TYPE_INTEGER;
    	if (type == Types::type_f32) 	return INTERNAL_TYPE_DECIMAL;
    	if (type == Types::type_f64) 	return INTERNAL_TYPE_DECIMAL;
    	if (type == Types::type_any) 	return INTERNAL_TYPE_ANY;
    	return INTERNAL_TYPE_UNDEFINED;
    }

    static bool is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
    	if (type_to == Types::type_any) {
    		return true;
    	} else if (type_from->is_primitive && type_to->is_primitive) {
    		if (type_to == Types::type_bool) return true;
    		else if (type_from->is_signed == type_to->is_signed) {
    			return type_to->byte_size >= type_from->byte_size;
    		} else if (!type_from->is_signed && type_to->is_signed) {
    			return type_to->byte_size > type_from->byte_size;
    		} else return false;
    	} else if (!type_from->is_primitive && !type_to->is_primitive) {
    		if (type_from->typedef_type == AST_TYPEDEF_ARRAY && type_to->typedef_type == AST_TYPEDEF_STRUCT) {
    			auto array_type = static_cast<Ast_Array_Type*>(type_from);
    			auto struct_type = static_cast<Ast_Struct_Type*>(type_to);
    			if (struct_type->is_slice) {
    				// TODO: we're assuming both base types are instance of types
    				// (we should check that once in the whole code!)
    				auto slice_type = static_cast<Ast_Slice_Type*>(struct_type);
    				auto base_type1 = static_cast<Ast_Type_Instance*>(array_type->base);
    				auto base_type2 = slice_type->get_typed_base();
    				return ast_types_are_equal(base_type1, base_type2);
    			}
    		}
    	}
    	return false;
    }
};
