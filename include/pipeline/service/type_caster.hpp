#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

struct Type_Caster {
    Build_Context* context = NULL;

    Type_Table* type_table = NULL;

    void init (Build_Context* c) {
        this->context = c;

        this->type_table = c->type_table;
    }

    bool try_implicid_cast (Ast_Type* type_from, Ast_Type* type_to, Ast_Expression** exp_ptr) {
        if (!type_from || !type_to) return false;
        if (type_from == type_to) return true;

        if (type_from->typedef_type == AST_TYPEDEF_TUPLE) {
            auto tuple_type_from = static_cast<Ast_Tuple_Type*>(type_from);

            if (type_to->typedef_type != AST_TYPEDEF_TUPLE) {
                if (tuple_type_from->types.size != 1) return false;

                auto exp = (*exp_ptr);
                if (exp->exp_type == AST_EXPRESSION_COMMA_SEPARATED) {
                    auto comma_separated = static_cast<Ast_Comma_Separated*>(exp);
                    exp_ptr = (comma_separated->expressions.data + 0);
                }

                auto type_exp = tuple_type_from->types[0];
                assert(type_exp->exp_type == AST_EXPRESSION_TYPE);
                auto type_from_in_tuple = static_cast<Ast_Type*>(type_exp);

                return this->try_implicid_cast(type_from_in_tuple, type_to, exp_ptr);
            }
        }

        if (this->can_be_implicidly_casted(type_from, type_to)) {
            auto cast = new Ast_Cast(*exp_ptr, type_to);
            cast->path = (*exp_ptr)->path;
            cast->inferred_type = type_to;
            (*exp_ptr) = cast;
            return true;
        }

        return false;
    }

    bool try_bidirectional_implicid_cast (Ast_Expression** exp_ptr1, Ast_Expression** exp_ptr2) {
        auto type1 = (*exp_ptr1)->inferred_type;
        auto type2 = (*exp_ptr2)->inferred_type;

        if (type1 == type2) return true;

        bool success = this->try_implicid_cast(type1, type2, exp_ptr1);
        if (success) return true;

        success = this->try_implicid_cast(type2, type1, exp_ptr2);
        if (success) return true;

        return false;
    }

    bool can_be_implicidly_casted (Ast_Type* type_from, Ast_Type* type_to) {
        if (type_from->is_primitive && type_to == this->type_table->type_bool) return true;

        if (type_from->is_primitive && type_to->is_primitive) {
            if (type_from->is_number && type_to->is_number) {
                if (type_from->is_signed == type_to->is_signed) {
                    return type_from->byte_size <= type_to->byte_size;
                } else {
                    // @TODO: handle implicid casts from - to signed/unsigned
                }
            }
        }

        return false;
    }

    Ast_Type* get_container_signed (Ast_Type* unsigned_type) {
        if (unsigned_type == this->type_table->type_u8) {
            return this->type_table->type_s16;
        } else if (unsigned_type == this->type_table->type_u16) {
            return this->type_table->type_s32;
        } else if (unsigned_type == this->type_table->type_u32) {
            return this->type_table->type_s64;
        } else if (unsigned_type == this->type_table->type_u64) {
            return this->type_table->type_s64;
        } else return unsigned_type;
    }
};
