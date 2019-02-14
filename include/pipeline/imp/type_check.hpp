#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

#include "pipeline/service/type_inferrer.hpp"

struct Type_Check : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    Type_Inferrer* inferrer = new Type_Inferrer();

    Type_Check () : Compiler_Pipe("Type Check") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Declaration* decl) {
        Ast_Navigator::ast_handle(decl);

        if (!decl->type && !decl->expression) {
            this->print_error(decl, "Declarations must either have a value or a type");
            return;
        }

        if (!decl->type) {
            this->inferrer->infer(decl->expression);
            decl->type = decl->expression->inferred_type;
        } else if (!decl->expression) {
            this->inferrer->infer(decl->type);
            if (decl->type->inferred_type != Types::type_type) {
                //this->print_error(decl->type, "Type of declaration must be a type instance");
            }
        } else {
            decl->expression->inferred_type = static_cast<Ast_Type*>(decl->type);
        }
    }

    void ast_handle (Ast_Expression* exp) {
        Ast_Navigator::ast_handle(exp);
        this->inferrer->infer(exp);
    }

    void ast_handle (Ast_Binary* binary) {
        Ast_Navigator::ast_handle(binary);
        this->inferrer->infer(binary);
    }

    void ast_handle (Ast_Unary* unary) {
        if (unary->unary_op != AST_UNARY_REFERENCE) {
            Ast_Navigator::ast_handle(unary);
        }
    }
};