#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Compute_Sizes : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {

    Compute_Sizes () : Compiler_Pipe("Compute Sizes") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Struct_Type* struct_type) {
        Ast_Navigator::ast_handle(struct_type);

        if (struct_type->struct_flags & STRUCT_FLAG_SIZED) return;

        struct_type->byte_size = 0;
        struct_type->byte_padding = 0;
        for (size_t i = 0; i < struct_type->scope.statements.size(); i++) {
            auto stm = struct_type->scope.statements[i];
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                assert(decl->type->exp_type == AST_EXPRESSION_TYPE);

                auto decl_type = static_cast<Ast_Type*>(decl->type);
                if (!(struct_type->struct_flags & STRUCT_FLAG_SIZED)) return;

                decl->attribute_byte_offset = struct_type->byte_size;

                struct_type->byte_size += decl_type->byte_size;
                if ((struct_type->byte_size % 8) > 0) {
                    auto padding = 8 - (struct_type->byte_size % 8);
                    struct_type->byte_padding += padding;
                    struct_type->byte_size += padding;
                }
            } else assert(false);
        }

        struct_type->struct_flags |= STRUCT_FLAG_SIZED;
    }

    void ast_handle (Ast_Scope* scope) {
        Ast_Navigator::ast_handle(scope);

        if (!(scope->scope_flags & SCOPE_FLAG_IS_IMPERATIVE)) return;
        if (scope->scope_flags & SCOPE_FLAG_IS_SIZED) return;

        for (size_t i = 0; i < scope->statements.size(); i++) {
            auto stm = scope->statements[i];

            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
                auto decl_type = static_cast<Ast_Type*>(decl->type);

                scope->stack_byte_size += decl_type->byte_size;
                if ((scope->stack_byte_size % 8) > 0) {
                    auto padding = 8 - (scope->stack_byte_size % 8);
                    scope->stack_byte_padding += padding;
                    scope->stack_byte_size += padding;
                }
            }
        }

        scope->scope_flags |= SCOPE_FLAG_IS_SIZED;
    }

    void ast_handle (Ast_Pointer_Type* ptr_type) {
        if (ptr_type->byte_size > 0) return;
        Ast_Navigator::ast_handle(ptr_type);

        ptr_type->byte_size = this->context->target_arch->register_size;
    }

    void ast_handle (Ast_Function_Type* func_type) {
        if (func_type->byte_size > 0) return;
        Ast_Navigator::ast_handle(func_type);

        func_type->byte_size = this->context->target_arch->register_size;
    }
};
