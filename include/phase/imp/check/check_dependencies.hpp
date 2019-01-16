#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"

#include "util/logger.hpp"

struct Check_Dependencies : Phase, Ast_Navigator {
    bool errors_found = false;

    Check_Dependencies() : Phase("Check Dependencies", CE_MODULE_CHECK_DEPENDENCIES) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        this->ast_handle(global_scope);

        if (this->errors_found) {
            Events::trigger(CE_COMPILER_ERROR);
        } else this->push(global_scope);
    }

    void ast_handle (Ast_Declaration* decl) {
        if (!decl->type) {
            Logger::internal(decl, "Type of declaration could not be inferred");
            this->errors_found = true;
        } else if (decl->type->exp_type != AST_EXPRESSION_TYPE_INSTANCE) {
            Logger::error(decl, "Type of declaration is not a type instance");
            this->errors_found = true;
        }
    }

    void ast_handle (Ast_Ident* ident) {
        if (!ident->declaration) {
            Logger::error(ident, "Identifier '%s' has no declaration", ident->name);
            this->errors_found = true;
        }
    }

    void ast_handle (Ast_Expression* exp) {
        Ast_Navigator::ast_handle(exp);
        if (!exp->inferred_type) {
            Logger::internal(exp, "Expression type could not be inferred");
            this->errors_found = true;
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
