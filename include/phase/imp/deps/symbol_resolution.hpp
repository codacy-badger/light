#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

struct Symbol_Resolution : Phase, Ast_Navigator {
    Ast_Scope* current_scope = NULL;

    size_t symbols_resolved = 0;

    Symbol_Resolution() : Phase("External Symbol Resolution") { /* empty */ }

    void handle_main_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        this->ast_handle(module->global_scope);

        Events::trigger(this->event_to_id, module);
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            auto decl = this->current_scope->find_declaration(ident->name, true, true, true);
            if (decl) {
                ident->declaration = decl;
                this->symbols_resolved++;
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

	void print_extra_metrics() {
		print_extra_metric("Symbols resolved", "%zd", this->symbols_resolved);
	}
};
