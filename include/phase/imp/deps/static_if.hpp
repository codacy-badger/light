#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"

struct Static_If : Phase, Ast_Navigator {
    bool static_ifs_found = false;

    Static_If() : Phase("Static If", CE_MODULE_RESOLVE_IFS) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        this->static_ifs_found = false;
        Ast_Navigator::ast_handle(global_scope);

        if (this->static_ifs_found) {
            Events::trigger(CE_MODULE_RESOLVE_IMPORTS, global_scope);
        } else this->push(global_scope);
    }

    void ast_handle (Ast_Static_If* if_dir) {
        if_dir->remove_from_scope = true;
        this->static_ifs_found = true;

        if (if_dir->stm_if->condition->exp_type == AST_EXPRESSION_LITERAL) {
            auto literal = static_cast<Ast_Literal*>(if_dir->stm_if->condition);
            if (literal->uint_value != 0) {
                this->add_stms_after(if_dir, if_dir->stm_if->then_scope);
            } else if (if_dir->stm_if->else_scope != NULL) {
                this->add_stms_after(if_dir, if_dir->stm_if->else_scope);
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
};
