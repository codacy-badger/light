#pragma once

#include "steps/async_pipe.hpp"

#include "utils/ast_ref_navigator.hpp"
#include "ast/cloner.hpp"

struct Symbol_Resolution_Step : Async_Pipe, Ast_Ref_Navigator {
    Symbol_Resolution_Step() : Async_Pipe("Resolve Symbols") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        Ast_Ref_Navigator::ast_handle(&stm);
        this->pipe_out(in);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            ident->declaration = this->current_scope()->find_declaration(ident->name, true, true, true);
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
