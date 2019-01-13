#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

struct Static_If : Phase, Ast_Navigator {
    Ast_Scope* current_scope = NULL;
    bool static_ifs_found = false;

    Static_If() : Phase("Static If") { /* empty */ }

    void handle_main_event (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        this->static_ifs_found = false;
        this->ast_handle(module->global_scope);
        if (this->static_ifs_found) {
            Events::trigger(CE_MODULE_RESOLVE_IMPORTS, module);
        } else {
            Events::trigger(this->event_to_id, module);
        }
    }

    void ast_handle (Ast_Directive_If* if_dir) {
        if_dir->remove_from_scope = true;
        this->static_ifs_found = true;

        if (if_dir->stm_if->condition->exp_type == AST_EXPRESSION_IDENT) {
            auto ident = static_cast<Ast_Ident*>(if_dir->stm_if->condition);
            if (ident->declaration->is_constant) {
                auto value = ident->declaration->expression;
                if (value->exp_type == AST_EXPRESSION_LITERAL) {
                    auto literal = static_cast<Ast_Literal*>(value);
                    if (literal->uint_value != 0) {
                        this->add_stms_after(if_dir, if_dir->stm_if->then_scope);
                    } else if (if_dir->stm_if->else_scope != NULL) {
                        this->add_stms_after(if_dir, if_dir->stm_if->else_scope);
                    }
                }
            }
        } else {
            printf("Static if condition can only be constant literals!\n");
            exit(1);
        }
    }

    void add_stms_after (Ast_Statement* stm, Ast_Scope* scope) {
        auto stms = &this->current_scope->statements;
        auto new_stms = &scope->statements;

        auto current_stm = std::find(stms->begin(), stms->end(), stm);
        stms->insert(current_stm + 1, new_stms->begin(), new_stms->end());
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;
        Ast_Navigator::ast_handle(scope);
        this->current_scope = tmp;
    }
};
