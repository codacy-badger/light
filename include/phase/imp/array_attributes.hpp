#pragma once

#include "phase/phase.hpp"
#include "phase/ast_ref_navigator.hpp"

#include "ast/ast_factory.hpp"
#include "compiler_events.hpp"

struct Array_Attributes : Phase, Ast_Ref_Navigator {

    Array_Attributes() : Phase("Array Attributes", CE_MODULE_REPLACE_ARRAY_ATTRIBUTES) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Ref_Navigator::ast_handle(&global_scope);

        this->push(global_scope);
    }

    void unique (Ast_Binary** binop_ptr) {
        auto binop = (*binop_ptr);

        if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
            auto attr_ident = static_cast<Ast_Ident*>(binop->rhs);

            if (binop->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
                auto array_type = static_cast<Ast_Array_Type*>(binop->lhs->inferred_type);
                if (strcmp(attr_ident->name, "data") == 0) {
                    auto pointer_type = Ast_Factory::pointer_type(array_type->base);
                    (*binop_ptr) = (Ast_Binary*) Ast_Factory::ref(binop->lhs, pointer_type);
                } else if (strcmp(attr_ident->name, "length") == 0) {
                    (*binop_ptr) = (Ast_Binary*) Ast_Factory::literal(binop->location, array_type->length_uint);
                }
            }
        }
    }
};
