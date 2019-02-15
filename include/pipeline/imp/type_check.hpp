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
            this->error(decl, "Declarations must either have a value or a type");
            return;
        }

        if (!decl->type && decl->expression) {
            decl->type = decl->expression->inferred_type;
        } else if (decl->type && decl->expression) {
            // @TODO match both inferred types & insert implicid cast if needed
            decl->expression->inferred_type = static_cast<Ast_Type*>(decl->type);
        }
    }

    void ast_handle (Ast_Expression* exp) {
        Ast_Navigator::ast_handle(exp);
        this->inferrer->infer(exp);
    }

    void ast_handle (Ast_Binary* binary) {
        if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
            this->ast_handle(binary->lhs);

            assert(binary->lhs->inferred_type);
            assert(binary->rhs->exp_type == AST_EXPRESSION_IDENT);
            auto ident = static_cast<Ast_Ident*>(binary->rhs);

            this->bind_attribute_or_error(binary->lhs->inferred_type, ident, &binary->lhs);
            assert(ident->declaration);

            this->ast_handle(binary->rhs);
        } else Ast_Navigator::ast_handle(binary);
    }

    void ast_handle (Ast_Unary* unary) {
        if (unary->unary_op != AST_UNARY_REFERENCE) {
            Ast_Navigator::ast_handle(unary);
        }
    }

    void bind_attribute_or_error (Ast_Type* type, Ast_Ident* ident, Ast_Expression** exp_ptr) {
        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                auto struct_type = static_cast<Ast_Struct_Type*>(type);
                auto attr_decl = struct_type->find_attribute(ident->name);
                if (attr_decl) {
                    ident->declaration = attr_decl;
                } else {
                    this->error(ident, "Struct '%s' has no attribute named '%s'", type->name, ident->name);
                }
                break;
            }
            case AST_TYPEDEF_POINTER: {
                while (type->typedef_type == AST_TYPEDEF_POINTER) {
                    auto tmp = static_cast<Ast_Pointer_Type*>(type);
                    assert(tmp->base->exp_type == AST_EXPRESSION_TYPE);
                    type = tmp->typed_base;

                    auto tmp2 = (Ast_Ident*) new Ast_Unary(AST_UNARY_DEREFERENCE, *exp_ptr);
                    tmp2->location = (*exp_ptr)->location;
                    (*exp_ptr) = tmp2;
                }

                this->bind_attribute_or_error(type, ident, exp_ptr);

                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                this->error(ident, "Attribute access cannot be performed on function types");
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->error(ident, "TODO");
                break;
            }
        }
    }
};
