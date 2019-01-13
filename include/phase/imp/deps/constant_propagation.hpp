#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_ref_navigator.hpp"
#include "ast/ast_cloner.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

#include "util/logger.hpp"

struct Constant_Propagation : Phase, Ast_Ref_Navigator {
    Constant_Propagation () : Phase ("Constant Propagation", CE_MODULE_PROPAGATE_CONSTANTS) { /* empty */ }

    void handle_main_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        Ast_Ref_Navigator::ast_handle(&module->global_scope);

        this->push(module);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (ident->declaration && ident->declaration->is_constant) {
            auto value = ident->declaration->expression;

            if (value->exp_type == AST_EXPRESSION_FUNCTION
                    || value->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                (*ident_ptr) = reinterpret_cast<Ast_Ident*>(value);
            } else {
                auto cloned_value = Ast_Cloner::clone(value);
                (*ident_ptr) = reinterpret_cast<Ast_Ident*>(cloned_value);
            }
        }
    }
};
