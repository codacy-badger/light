#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/types.hpp"

struct Type_Caster {
    Build_Context* context = NULL;

    void init (Build_Context* c) { this->context = c; }

    bool try_implicid_cast (Ast_Type* type_from, Ast_Type* type_to, Ast_Expression**) {
        if (type_from == type_to) return true;

        printf("Try implicid cast: '%s' -> '%s'\n", type_from->name, type_to->name);
        return false;
    }

    bool try_bidirectional_implicid_cast (Ast_Expression** exp_ptr1, Ast_Expression** exp_ptr2) {
        auto type1 = (*exp_ptr1)->inferred_type;
        auto type2 = (*exp_ptr2)->inferred_type;

        if (type1 == type2) return true;

        printf("Try bidirectional implicid cast: '%s' <-> '%s'\n", type1->name, type2->name);

        return false;
    }
};
