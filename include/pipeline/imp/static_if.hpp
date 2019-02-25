#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Static_If : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Ast_Scope* current_global_scope = NULL;
    Ast_Scope* current_scope = NULL;

    Async_Queue<Ast_Statement*>* flow_back = NULL;
    bool has_generated_new_internal_code = false;

    Static_If(Async_Queue<Ast_Statement*>* flow_back) : Compiler_Pipe("Static If") {
        this->flow_back = flow_back;
    }

    void handle (Ast_Statement* global_statement) {
        this->current_global_scope = global_statement->parent_scope;
        this->current_scope = global_statement->parent_scope;
        this->has_generated_new_internal_code = false;
        Ast_Navigator::ast_handle(global_statement);

        if (global_statement->stm_type != AST_STATEMENT_STATIC_IF) {
            if (this->has_generated_new_internal_code) {
                flow_back->push(global_statement);
            } else {
                global_statement->stm_flags |= STM_FLAG_STATIC_IFS_RESOLVED;
                this->push_out(global_statement);
            }
        }
    }

    void ast_handle (Ast_Static_If* static_if) {
        auto _if = static_if->stm_if;

        auto condition_value = this->get_value_as_bool(_if->condition);
        if (condition_value) {
            this->resolve_static_if(static_if, _if->then_body);
        } else if (_if->else_body) {
            this->resolve_static_if(static_if, _if->else_body);
        }

        if (this->current_scope != this->current_global_scope) {
            this->current_scope->remove(static_if);
        }
    }

    void resolve_static_if (Ast_Static_If* static_if, Ast_Scope* scope_to_merge) {
        auto static_if_location = static_if->parent_scope->find(static_if);
        static_if->parent_scope->add(static_if_location, scope_to_merge);

        if (this->current_scope == this->current_global_scope) {
            for (auto stm : scope_to_merge->statements) {
                flow_back->push(stm);
            }
        } else this->has_generated_new_internal_code = true;
    }

    void ast_handle (Ast_Declaration* decl) {
        // INFO: we don't want to resolve static ifs for declaration types, since
        // those types could be values of other global declaration and we would
        // be falsely responsible of resolving those ifs.
        if (decl->value) Ast_Navigator::ast_handle(decl->value);
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;
        Ast_Navigator::ast_handle(scope);
        this->current_scope = tmp;
    }

    bool get_value_as_bool (Ast_Expression* exp) {
        if (exp->exp_type == AST_EXPRESSION_LITERAL) {
            auto literal = static_cast<Ast_Literal*>(exp);
            return literal->uint_value;
        } else if (exp->exp_type == AST_EXPRESSION_IDENT) {
            auto ident = static_cast<Ast_Ident*>(exp);
            assert(ident->declaration);
            assert(ident->declaration->is_constant);
            return this->get_value_as_bool(ident->declaration->value);
        } else {
            this->context->error(exp, "Static IF condition can only be literal or pre-declared constant");
            return false;
        }
    }
};
