#pragma once

#include <vector>
#include <map>

#include "ast/ast.hpp"

enum Internal_Type : uint8_t {
    INTERNAL_TYPE_UNDEFINED,

    INTERNAL_TYPE_VOID = 0,
    INTERNAL_TYPE_BOOL,
    INTERNAL_TYPE_INTEGER,
    INTERNAL_TYPE_DECIMAL,
    INTERNAL_TYPE_STRING,
    INTERNAL_TYPE_ANY,
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

    // TODO: solve this: either we unique all types or we make deep comparisons
    static bool equal (Ast_Type_Instance* type_inst1, Ast_Type_Instance* type_inst2) {
        //return type_inst1 == type_inst2;
        if (type_inst1 == type_inst2) return true;
        else {
            if (type_inst1->typedef_type != type_inst2->typedef_type) return false;
            switch (type_inst1->typedef_type) {
                case AST_TYPEDEF_STRUCT: return false;
                case AST_TYPEDEF_POINTER: {
                    auto ptr_type1 = static_cast<Ast_Pointer_Type*>(type_inst1);
                    auto ptr_type2 = static_cast<Ast_Pointer_Type*>(type_inst2);
                    auto base_type1 = static_cast<Ast_Type_Instance*>(ptr_type1->base);
                    auto base_type2 = static_cast<Ast_Type_Instance*>(ptr_type2->base);
                    return Types::equal(base_type1, base_type2);
                }
                case AST_TYPEDEF_ARRAY: {
                    auto arr_type1 = static_cast<Ast_Array_Type*>(type_inst1);
                    auto arr_type2 = static_cast<Ast_Array_Type*>(type_inst2);

                    if (arr_type1->get_length() != arr_type2->get_length()) return false;

                    auto base_type1 = static_cast<Ast_Type_Instance*>(arr_type1->base);
                    auto base_type2 = static_cast<Ast_Type_Instance*>(arr_type2->base);
                    return Types::equal(base_type1, base_type2);
                }
                case AST_TYPEDEF_FUNCTION: {
                    auto func_type1 = static_cast<Ast_Function_Type*>(type_inst1);
                    auto func_type2 = static_cast<Ast_Function_Type*>(type_inst2);
                    return Types::function_types_equal(func_type1, func_type2);
                }
                default: abort();
            }
        }
    }

    static bool function_types_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
        if (func_type1->arg_decls.size() != func_type2->arg_decls.size()) return false;

        for (size_t i = 0; i < func_type1->arg_decls.size(); i++) {
            auto arg_type1 = static_cast<Ast_Type_Instance*>(func_type1->arg_decls[i]->type);
            auto arg_type2 = static_cast<Ast_Type_Instance*>(func_type2->arg_decls[i]->type);
            if (!Types::equal(arg_type1, arg_type2)) return false;
        }

        auto ret_type1 = static_cast<Ast_Type_Instance*>(func_type1->ret_type);
        auto ret_type2 = static_cast<Ast_Type_Instance*>(func_type2->ret_type);
        return Types::equal(ret_type1, ret_type2);
    }

    static bool try_convert (Ast_Expression** exp_ptr,
            Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
        return Types::try_implicid_cast(exp_ptr, type_from, type_to)
            || Types::try_coercion(exp_ptr, type_from, type_to);
    }

    static bool try_implicid_cast (Ast_Expression** exp_ptr,
            Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
    	if (type_from->is_primitive && type_to->is_primitive) {
    		if (type_to == Types::type_bool) {
                auto cast = new Ast_Cast((*exp_ptr), type_to);
        		cast->location = (*exp_ptr)->location;
                cast->inferred_type = type_to;
                (*exp_ptr) = cast;
                return true;
    		} else if (type_from->is_signed == type_to->is_signed) {
                if (type_to->byte_size >= type_from->byte_size) {
                    auto cast = new Ast_Cast((*exp_ptr), type_to);
            		cast->location = (*exp_ptr)->location;
                    cast->inferred_type = type_to;
                    (*exp_ptr) = cast;
                    return true;
                }
    		} else if (!type_from->is_signed && type_to->is_signed) {
                if (type_to->byte_size > type_from->byte_size) {
                    auto cast = new Ast_Cast((*exp_ptr), type_to);
            		cast->location = (*exp_ptr)->location;
                    cast->inferred_type = type_to;
                    (*exp_ptr) = cast;
                    return true;
                }
    		}
    	}
        return false;
    }

    static bool try_implicid_cast (Ast_Expression** exp_ptr, Ast_Type_Instance* type_to) {
    	return Types::try_implicid_cast(exp_ptr, (*exp_ptr)->inferred_type, type_to);
    }

    static bool try_coercion (Ast_Expression**, Ast_Type_Instance*, Ast_Type_Instance*) {
        return false;
    }
};
