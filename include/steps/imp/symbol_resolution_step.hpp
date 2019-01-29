#pragma once

#include "steps/step.hpp"

#include "utils/ast_ref_navigator.hpp"
#include "ast/cloner.hpp"

struct Symbol_Resolution_Step : Step<>, Ast_Ref_Navigator {
    Symbol_Resolution_Step() : Step("Resolve Symbols") { /* empty */ }

    void run (Ast_Statement* stm) {
        Ast_Ref_Navigator::ast_handle(&stm);
        this->push_out(stm);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            ident->declaration = ident->scope->find_declaration(ident->name, true, true, true);
        }
    }

    void ast_handle (Ast_Binary** binary_ptr) {
        // @Info we can't resolve atrtibutes just yet, since we
        // still don't have type information on expressions
        if ((*binary_ptr)->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Ref_Navigator::ast_handle(binary_ptr);
        }
    }
};
