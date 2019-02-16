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
            this->context->shutdown();
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
                this->context->error(decl->expression, "Value cannot be implicitly casted from '%s' to '%s'",
                    decl->expression->inferred_type->name, type->name);
                this->context->shutdown();
                return;
            }
        }
    }

    void ast_handle (Ast_Expression** exp_ptr) {
        Ast_Ref_Navigator::ast_handle(exp_ptr);
        this->inferrer->infer(*exp_ptr);
    }

    void ast_handle (Ast_Function_Call** call_ptr) {
        auto call = (*call_ptr);

        Ast_Ref_Navigator::ast_handle(call_ptr);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        auto min_arg_count = func_type->count_arguments_without_defaults();
        auto max_arg_count = func_type->arg_decls.size();

        if (call->arguments->unnamed.size() < min_arg_count) {
            this->context->error(call, "Too few arguments for function call, should have at least %zd", min_arg_count);
            this->context->shutdown();
            return;
        }

        if (call->arguments->unnamed.size() > max_arg_count) {
            this->context->error(call, "Too many arguments for function call, should have at most %zd", max_arg_count);
            this->context->shutdown();
            return;
        }

        call->arguments->unnamed.reserve(max_arg_count);
        for (size_t i = call->arguments->unnamed.size(); i < max_arg_count; i++) {
            call->arguments->unnamed.push_back(NULL);
        }

        for (auto entry : call->arguments->named) {
            auto arg_index = func_type->get_argument_index(entry.first);
            if (arg_index != INVALID_ARGUMENT_INDEX) {
                auto existing_value = call->arguments->get_unnamed_value(arg_index);
                if (existing_value) {
                    this->context->error(call, "Multiple values provided for argument '%s'", entry.first);
                    this->context->shutdown();
                    return;
                } else {
                    call->arguments->unnamed[arg_index] = entry.second;
                }
            } else {
                //
                // @Bug @TODO this could produce incorrect error messages, since
                // function types contain the argument names, but are also uniqued!
                // possible solution: the argument names should be stored in the Ast_Function
                // nodes, but we should be sure that assert(arg_names.size() == arg_decls.size())
                //
                this->context->error(call, "Function has no argument named '%s'", entry.first);
                this->context->shutdown();
                return;
            }
        }
        call->arguments->named.clear();

        for (size_t i = min_arg_count; i < func_type->arg_decls.size(); i++) {
            auto existing_value = call->arguments->get_unnamed_value(i);
            if (!existing_value) {
                auto arg_decl = func_type->arg_decls[i];
                assert(arg_decl->expression);

                call->arguments->unnamed[i] = arg_decl->expression;
            }
        }

        for (size_t i = 0; i < call->arguments->unnamed.size(); i++) {
            auto arg_decl = func_type->arg_decls[i];
            auto value = call->arguments->unnamed[i];
            assert(arg_decl->type->exp_type == AST_EXPRESSION_TYPE);

            auto success = this->caster->try_implicid_cast(value->inferred_type,
                arg_decl->typed_type, &(call->arguments->unnamed[i]));
            if (!success) {
                this->context->error(value, "Value cannot be implicitly casted from '%s' to '%s'",
                    value->inferred_type->name, arg_decl->typed_type->name);
                this->context->shutdown();
                return;
            }
        }
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
                    this->context->shutdown();
                    return;
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
                this->context->shutdown();
                return;
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->context->error(ident, "TODO");
                this->context->shutdown();
                return;
                break;
            }
        }
    }
};
