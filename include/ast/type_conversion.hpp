#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"
#include "ast/ast_factory.hpp"

struct Type_Conversion {
    static bool try_convert (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr,
            Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
        return Type_Conversion::try_implicid_cast(exp_ptr, type_from, type_to)
            || Type_Conversion::try_coercion(to_prepend, exp_ptr, type_from, type_to);
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
        return Type_Conversion::try_implicid_cast(exp_ptr, (*exp_ptr)->inferred_type, type_to);
    }

    static bool try_coercion (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr,
            Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
        if (type_from->typedef_type == AST_TYPEDEF_ARRAY) {
            auto type_from_array = static_cast<Ast_Array_Type*>(type_from);
            if (type_from_array->base == Types::type_char && type_to == Types::type_string) {
                Type_Conversion::coerce_char_array_to_string(to_prepend, exp_ptr);
                return true;
            }
        }
        return false;
    }

    static void coerce_char_array_to_string (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr) {
        auto str = (*exp_ptr);

        char tmp_name[TMP_NAME_SIZE];
        static size_t string_coercion_count = 0;
        sprintf_s(tmp_name, TMP_NAME_SIZE, "$tmp$str$%zd", string_coercion_count++);

        // $tmp : string;
        auto string_declaration = Ast_Factory::declaration(str->location, tmp_name, Types::type_string);
        //string_declaration->scope = this->current_scope;
        to_prepend->push_back(string_declaration);

        // $tmp.length = array.length;
        auto tmp_ident1 = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);

        auto tmp_length = Ast_Factory::attr(tmp_ident1, "length", Types::type_u64);
        auto str_length = Ast_Factory::attr(str, "length", Types::type_u64);
        auto assign_length = Ast_Factory::assign(tmp_length, str_length);
        to_prepend->push_back(assign_length);

        // $tmp.data = array.data;
        auto tmp_ident2 = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);

        auto tmp_data = Ast_Factory::attr(tmp_ident2, "data");
        auto str_data = Ast_Factory::attr(str, "data");
        auto assign_data = Ast_Factory::assign(tmp_data, str_data);
        to_prepend->push_back(assign_data);

        (*exp_ptr) = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);
    }
};
