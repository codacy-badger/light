#pragma once

#include "phase/async_phase.hpp"
#include "phase/ast_ref_navigator.hpp"

#include "module.hpp"
#include "ast/ast_cloner.hpp"
#include "compiler_events.hpp"

struct Symbol_Resolution : Async_Phase, Ast_Ref_Navigator {
    size_t symbols_resolved = 0;

    Symbol_Resolution() : Async_Phase("Symbol Resolution", CE_MODULE_RESOLVE_SYMBOLS) { /* empty */ }

    void handle_main_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        Ast_Ref_Navigator::ast_handle(&module->global_scope);

        Events::trigger(this->event_to_id, module);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            auto decl = this->current_scope->find_declaration(ident->name, true, true, true);
            if (decl) {
                this->symbols_resolved++;

                if (decl->is_constant) {
                    auto value = decl->expression;

                    if (value->exp_type == AST_EXPRESSION_FUNCTION
                            || value->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
                        (*ident_ptr) = reinterpret_cast<Ast_Ident*>(value);
                    } else {
                        auto cloned_value = Ast_Cloner::clone(value);
                        (*ident_ptr) = reinterpret_cast<Ast_Ident*>(cloned_value);
                    }
                } else ident->declaration = decl;
            }
        }
    }

    void ast_handle (Ast_Binary** binary_ptr) {
        // @Info we can't resolve atrtibutes just yet, since we
        // still don't have type information on expressions
        if ((*binary_ptr)->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Ref_Navigator::ast_handle(binary_ptr);
        }
    }

	void print_extra_metrics() {
		print_extra_metric("Symbols resolved", "%zd", this->symbols_resolved);
	}
};
