#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include "utils/array.hpp"

struct Resolve_Idents : Compiler_Pipe<Ast_Statement*>, Ast_Ref_Navigator {
    Array<Ast_Ident*> unresolved_idents;
    Ast_Scope* current_scope = NULL;

    bool resolving_inside_pointer = false;

    Resolve_Idents () : Compiler_Pipe("Resolved Idents") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        this->current_scope = global_statement->parent_scope;

        this->unresolved_idents.clear();
        Ast_Ref_Navigator::ast_handle(&global_statement);

        if (this->unresolved_idents.empty()) {
            global_statement->stm_flags |= STM_FLAG_IDENTS_RESOLVED;
            this->push_out(global_statement);
        } else this->requeue(global_statement);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        if (!ident->declaration) {
            auto decl = Ast_Utils::find_declaration(this->current_scope, ident->name, true, true);
            if (decl) {
                ident->declaration = decl;
            } else {
                this->unresolved_idents.push(ident);
                return;
            }
        }

        if (ident->declaration) {
            auto decl = ident->declaration;

            if (decl->is_global()) {
                if (decl->value->exp_type == AST_EXPRESSION_FUNCTION) {
                    auto func = static_cast<Ast_Function*>(decl->value);
                    if (Ast_Utils::is_ancestor_of(func->body, ident->scope)) {
                        // @INFO recursive resolution, so we can't depend on it
                        return;
                    }
                }

                if (!this->resolving_inside_pointer) {
                    if (!(decl->stm_flags & STM_FLAG_IDENTS_RESOLVED)) {
                        this->unresolved_idents.push(ident);
                        return;
                    }

                    if (!(decl->stm_flags & STM_FLAG_STATIC_IFS_RESOLVED)) {
                        this->unresolved_idents.push(ident);
                        return;
                    }
                }
            }
        }
    }

    void ast_handle (Ast_Binary** binop_ptr) {
        auto binop = (*binop_ptr);

        if (binop->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Ref_Navigator::ast_handle(binop_ptr);
        } else {
            Ast_Ref_Navigator::ast_handle(&binop->lhs);
            if (binop->lhs->exp_type == AST_EXPRESSION_IDENT) {
                auto ident = static_cast<Ast_Ident*>(binop->lhs);
                if (ident->declaration && ident->declaration->value != NULL) {
                    if (ident->declaration->value->exp_type == AST_EXPRESSION_IMPORT) {
                        auto attr = static_cast<Ast_Ident*>(binop->rhs);

                        auto import = static_cast<Ast_Import*>(ident->declaration->value);
                        if (!import->file_scope) {
                            this->unresolved_idents.push(attr);
                            return;
                        }

                        auto decl = Ast_Utils::find_declaration(import->file_scope, attr->name, false, false);
                        if (decl) {
                            attr->declaration = decl;
                            (*binop_ptr) = (Ast_Binary*) attr;
                            this->ast_handle((Ast_Ident**) binop_ptr);
                        } else {
                            this->unresolved_idents.push(attr);
                            return;
                        }
                    }
                }
            }
        }
    }

    void ast_handle (Ast_Unary** unary_ptr) {
        auto unary = (*unary_ptr);

        if (unary->unary_op == AST_UNARY_REFERENCE) {
            auto tmp = this->resolving_inside_pointer;
            this->resolving_inside_pointer = true;
            Ast_Ref_Navigator::ast_handle(unary_ptr);
            this->resolving_inside_pointer = tmp;
        } else Ast_Ref_Navigator::ast_handle(unary_ptr);
    }

    void ast_handle (Ast_Pointer_Type** ptr_type_ptr) {
        auto tmp = this->resolving_inside_pointer;
        this->resolving_inside_pointer = true;
        Ast_Ref_Navigator::ast_handle(ptr_type_ptr);
        this->resolving_inside_pointer = tmp;
    }

    void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;

        Ast_Ref_Navigator::ast_handle(scope);

        this->current_scope = tmp;
    }

    void on_shutdown () {
        if (this->context->has_error) return;
        
        if (!this->input_queue.empty()) {
            Array<Ast_Ident*> uniqued_idents;
            while (!this->input_queue.empty()) {
                auto stm = this->input_queue.pop();

                this->unresolved_idents.clear();
                Ast_Ref_Navigator::ast_handle(&stm);

                For (this->unresolved_idents) {
                    if (!uniqued_idents.contains(it)) {
                        uniqued_idents.push(it);
                    }
                }
            }

            this->context->error("Undefined identifiers found:");
            For (uniqued_idents) {
                this->context->error(it, "'%s'", it->name);
            }

            printf("\n");
            this->context->shutdown();
        }
    }
};
