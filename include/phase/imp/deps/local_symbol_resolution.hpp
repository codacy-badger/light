#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

struct Local_Symbol_Resolution : Phase, Ast_Navigator {
    Ast_Scope* current_scope = NULL;

    Local_Symbol_Resolution() : Phase("Local Symbol Resolution") { /* empty */ }

    void on_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        this->ast_handle(module->global_scope);

        Events::trigger(this->event_to_id, module);
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            auto decl = this->current_scope->find_local_declaration(ident->name, true);
            if (decl) {
                ident->declaration = decl;
            }
        }
    }

    void ast_handle (Ast_Binary* binary) {
        // @Info we can't resolve atrtibutes just yet, since we
        // still don't have type information on expressions
        if (binary->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Navigator::ast_handle(binary);
        }
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;
        Ast_Navigator::ast_handle(scope);
        this->current_scope = tmp;
    }
};
