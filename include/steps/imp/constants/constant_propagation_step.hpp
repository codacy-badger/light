#pragma once

#include "steps/step.hpp"

#include "utils/ast_ref_navigator.hpp"
#include "ast/factory.hpp"

struct Constant_Propagation_Step : Step<>, Ast_Ref_Navigator {
    bool* has_worked;

    Constant_Propagation_Step(bool* has_worked) : Step("Constant Propagation") {
        this->has_worked = has_worked;
    }

    void run (Ast_Statement* stm) {
        Ast_Ref_Navigator::ast_handle(&stm);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (ident->declaration && ident->declaration->is_constant) {
            this->replace_const_value(ident_ptr, ident->declaration->expression);
            (*has_worked) = true;
        }
    }

    void replace_const_value (Ast_Ident** ident_ptr, Ast_Expression* value) {
        if (value->exp_type == AST_EXPRESSION_FUNCTION || value->exp_type == AST_EXPRESSION_TYPE) {
            (*ident_ptr) = reinterpret_cast<Ast_Ident*>(value);
        } else {
            auto cloned_value = Ast_Cloner::clone(value);
            (*ident_ptr) = reinterpret_cast<Ast_Ident*>(cloned_value);
        }
    }
};
