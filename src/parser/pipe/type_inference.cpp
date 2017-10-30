#pragma once

#include "parser/pipe/type_inference.hpp"

#include "compiler.hpp"

void Type_Inference::on_statement(Ast_Statement* stm) {
    this->infer_type(stm);
    this->to_next(stm);
}

void Type_Inference::infer_type (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK:
            infer_type(static_cast<Ast_Block*>(stm));
            break;
        case AST_STATEMENT_DECLARATION:
            infer_type(static_cast<Ast_Declaration*>(stm));
            break;
        case AST_STATEMENT_RETURN:
            infer_type(static_cast<Ast_Return*>(stm));
            break;
        default: break;
    }
}

void Type_Inference::infer_type (Ast_Block* block) {
    for (auto stm : block->list) infer_type(stm);
}

void Type_Inference::infer_type (Ast_Declaration* decl) {
    if (decl->expression) infer_type(decl->expression);
    if (!decl->type) {
        if (decl->expression) {
            if (decl->expression->inferred_type) {
                printf("Type inferred!\n");
                decl->type->inferred_type = decl->expression->inferred_type;
            } else Light_Compiler::instance->report_error(decl, "Cannot infer type...");
        } else Light_Compiler::instance->report_error(decl, "Cannot infer type without an expression!");
    }
}

void Type_Inference::infer_type (Ast_Return* ret) {
    infer_type(ret->exp);
    // TODO: check if return type matches function's return type
}

void Type_Inference::infer_type (Ast_Expression* exp) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL:
            infer_type(static_cast<Ast_Literal*>(exp));
            break;
        default: break;
    }
}

void Type_Inference::infer_type (Ast_Literal* lit) {
    //TODO: smarter inference: use the smallest possible type
    switch (lit->literal_type) {
        case AST_LITERAL_INTEGER: {
            printf("Trying to infer literal type!\n");
            //TODO: fix usage of type instances
            break;
        }
        default: break;
    }
}
