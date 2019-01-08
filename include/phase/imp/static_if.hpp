#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

// @TODO rename this struct once we remove the old system
struct Static_If_Pipe : Async_Phase, Ast_Navigator {

    Static_If_Pipe() : Async_Phase("Static If", CE_MODULE_PARSED) { /* empty */ }

    void handle (void* data) {
        auto module = reinterpret_cast<Module*>(data);
        Ast_Navigator::ast_handle(module->global_scope);
    }

    void ast_handle (Ast_Directive_If* static_if) {
        printf("Static If directive: %p\n", static_if);
    }
};
