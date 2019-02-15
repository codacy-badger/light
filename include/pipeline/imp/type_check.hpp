#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include "pipeline/service/type_inferrer.hpp"
#include "pipeline/service/type_table.hpp"
#include "pipeline/service/type_caster.hpp"

struct Type_Check : Compiler_Pipe<Ast_Statement*>, Ast_Ref_Navigator {
    Type_Inferrer* inferrer = NULL;
    Type_Table* type_table = NULL;
    Type_Caster* caster = NULL;

    Type_Check () : Compiler_Pipe("Type Check") { /* empty */ }

    void init () {
        this->inferrer = this->context->type_inferrer;
        this->type_table = this->context->type_table;
        this->caster = this->context->type_caster;
    }

    void handle (Ast_Statement* global_statement) {
        Ast_Ref_Navigator::ast_handle(&global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Declaration* decl) {
        Ast_Ref_Navigator::ast_handle(decl);

        if (!decl->type && !decl->expression) {
            this->context->error(decl, "Declarations must either have a value or a type");
            return;
        }

        if (!decl->type && decl->expression) {
            decl->type = decl->expression->inferred_type;
        } else if (decl->type && decl->expression) {
            assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
            auto type = static_cast<Ast_Type*>(decl->type);

            auto success = this->caster->try_implicid_cast(decl->expression->inferred_type,
                type, &decl->expression);
            if (!success) {
                this->context->error(decl->expression, "Value cannot be casted to '%s'", type->name);
            }
        }
    }

    void ast_handle (Ast_Expression** exp_ptr) {
        Ast_Ref_Navigator::ast_handle(exp_ptr);
        this->inferrer->infer(*exp_ptr);
    }

    void ast_handle (Ast_Binary** binary_ptr) {
        auto binary = (*binary_ptr);

        if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
            this->ast_handle(&binary->lhs);

            assert(binary->lhs->inferred_type);
            assert(binary->rhs->exp_type == AST_EXPRESSION_IDENT);
            auto ident = static_cast<Ast_Ident*>(binary->rhs);

            this->bind_attribute_or_error(binary->lhs->inferred_type, ident, &binary->lhs);
            assert(ident->declaration);

            this->ast_handle(&binary->rhs);
        } else Ast_Ref_Navigator::ast_handle(binary_ptr);
    }

    void ast_handle (Ast_Unary** unary_ptr) {
        auto unary = (*unary_ptr);

        if (unary->unary_op != AST_UNARY_REFERENCE) {
            Ast_Ref_Navigator::ast_handle(unary_ptr);
        }
    }

    void ast_handle (Ast_Type** type_ptr) {
        this->type_table->unique(type_ptr);

        Ast_Ref_Navigator::ast_handle(type_ptr);
    }

    void bind_attribute_or_error (Ast_Type* type, Ast_Ident* ident, Ast_Expression** exp_ptr) {
        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                auto struct_type = static_cast<Ast_Struct_Type*>(type);
                auto attr_decl = struct_type->find_attribute(ident->name);
                if (attr_decl) {
                    ident->declaration = attr_decl;
                } else {
                    this->context->error(ident, "Struct '%s' has no attribute named '%s'", type->name, ident->name);
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
                this->context->error(ident, "Attribute access cannot be performed on function types");
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->context->error(ident, "TODO");
                break;
            }
        }
    }
};
