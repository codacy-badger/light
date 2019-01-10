#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

struct Check_Dependencies : Async_Phase, Ast_Navigator {

    Check_Dependencies() : Async_Phase("Check Dependencies") { /* empty */ }

    void on_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        this->ast_handle(module->global_scope);

        Events::trigger(this->event_to_id, module);
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            //printf("NO DECLARATION -> '%s'\n", ident->name);
        }
    }

    void ast_handle (Ast_Scope* scope) {
        // @TODO find duplicate declarations
        Ast_Navigator::ast_handle(scope);
    }

    void ast_handle (Ast_Binary* binary) {
        // @Info we can't resolve atrtibutes just yet, since we
        // still don't have type information on expressions
        if (binary->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Navigator::ast_handle(binary);
        }
    }
};
