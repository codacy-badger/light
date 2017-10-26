#pragma once

#include "parser/pipe/symbol_resolution.hpp"

void Symbol_Resolution::on_block_begin(Ast_Block* block) {
}

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    set<const char*, cmp_str> unresolved_symbols;
    if (!check_symbols(stm, &unresolved_symbols)) {
        printf("Found symbols inresolved!\n");
        for (auto symbol : unresolved_symbols) {
            printf("+ %s\n", symbol);
        }
    } else this->to_next(stm);
}

void Symbol_Resolution::on_block_end(Ast_Block* block) {
}

void Symbol_Resolution::on_finish () {
    this->try_finish();
}

bool Symbol_Resolution::check_symbols (Ast_Statement* stm, set<const char*, cmp_str>* sym) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION:
            return check_symbols(static_cast<Ast_Declaration*>(stm), sym);
        case AST_STATEMENT_BLOCK:
            return check_symbols(static_cast<Ast_Block*>(stm), sym);
        default: return false;
    }
}

bool Symbol_Resolution::check_symbols (Ast_Declaration* decl, set<const char*, cmp_str>* sym) {
    bool result = true;
    if (decl->type)         result &= check_symbols(decl->type, sym);
    if (decl->expression)   result &= check_symbols(decl->expression, sym);
    return result;
}

bool Symbol_Resolution::check_symbols (Ast_Block* block, set<const char*, cmp_str>* sym) {
    bool result = true;
    for (auto stm : block->list)
        result &= check_symbols(stm, sym);
    return result;
}

bool Symbol_Resolution::check_symbols (Ast_Expression* exp, set<const char*, cmp_str>* sym) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION:
            return check_symbols(static_cast<Ast_Function*>(exp), sym);
        case AST_EXPRESSION_TYPE_INSTANCE:
            return check_symbols(static_cast<Ast_Type_Instance*>(exp), sym);
        case AST_EXPRESSION_IDENT: {
            auto ident = static_cast<Ast_Ident*>(exp);
            sym->insert(ident->name);
            return false;
        }
        case AST_EXPRESSION_LITERAL: return true;
        default: return false;
    }
}

bool Symbol_Resolution::check_symbols (Ast_Function* fn, set<const char*, cmp_str>* sym) {
    bool result = true;
    result &= check_symbols(fn->type, sym);
    result &= check_symbols(fn->scope, sym);
    return result;
}

bool Symbol_Resolution::check_symbols (Ast_Type_Instance* ty_inst, set<const char*, cmp_str>* sym) {
    printf("Checking type instance...\n");
    return true;
}
