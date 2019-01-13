#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

#include "util/logger.hpp"

struct Check_Dependencies : Async_Phase, Ast_Navigator {
    bool errors_found = false;

    Check_Dependencies() : Async_Phase("Check Dependencies", CE_MODULE_CHECK_DEPENDENCIES) { /* empty */ }

    void handle_main_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        this->ast_handle(module->global_scope);

        if (this->errors_found) {
            Events::trigger(CE_COMPILER_ERROR);
        } else Events::trigger(this->event_to_id, module);
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            Logger::error(ident, "Identifier '%s' has no declaration", ident->name);
            this->errors_found = true;
        }
    }

    void ast_handle (Ast_Expression* exp) {
        if (!exp->inferred_type) {
            //Logger::error(exp, "Expression type could not be inferred");
            //this->errors_found = true;
        }
        Ast_Navigator::ast_handle(exp);
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
