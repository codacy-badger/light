#pragma once

#include "steps/sync_pipe.hpp"

#include "utils/ast_ref_navigator.hpp"
#include "ast/factory.hpp"

struct Constant_Propagation_Step : Sync_Pipe, Ast_Ref_Navigator {

    Constant_Propagation_Step() : Sync_Pipe("Constant Propagation") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        Ast_Ref_Navigator::ast_handle(&stm);
        this->pipe_out(in);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (ident->declaration && ident->declaration->is_constant) {
            this->replace_const_value(ident_ptr, ident->declaration->expression);
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
