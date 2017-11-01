#pragma once

#include "parser/pipe/Type_Checking.hpp"

#include "compiler.hpp"

void Type_Checking::on_statement(Ast_Statement* stm) {
    this->check_type(stm);
    this->to_next(stm);
}

void Type_Checking::check_type (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK:
            check_type(static_cast<Ast_Block*>(stm));
            break;
        case AST_STATEMENT_DECLARATION:
            check_type(static_cast<Ast_Declaration*>(stm));
            break;
        case AST_STATEMENT_RETURN:
            check_type(static_cast<Ast_Return*>(stm));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Block* block) {
    for (auto stm : block->list) check_type(stm);
}

void Type_Checking::check_type (Ast_Declaration* decl) {
    if (decl->expression) {
        check_type(decl->expression);

        if (!decl->type) {
            decl->type = decl->expression->inferred_type;
        } else {
            if (decl->type != decl->expression->inferred_type) {
                //Light_Compiler::instance->report_error(decl, "Type mismatch, wanted '%s' but got '%s'", "---", "---");
            }
        }
    } else if (!decl->type) {
        Light_Compiler::instance->report_error(decl, "Cannot infer type without an expression!");
    }
}

void Type_Checking::check_type (Ast_Return* ret) {
    check_type(ret->exp);
    // TODO: check if return type matches function's return type
}

void Type_Checking::check_type (Ast_Expression* exp) {
    switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION:
            check_type(static_cast<Ast_Type_Definition*>(exp));
            break;
        case AST_EXPRESSION_FUNCTION:
            check_type(static_cast<Ast_Function*>(exp));
            break;
        case AST_EXPRESSION_LITERAL:
            check_type(static_cast<Ast_Literal*>(exp));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Type_Definition* tydef) {
    switch (tydef->typedef_type) {
		case AST_TYPEDEF_FUNCTION:
            check_type(static_cast<Ast_Function_Type*>(tydef));
            break;
        case AST_TYPEDEF_STRUCT:
            check_type(static_cast<Ast_Struct_Type*>(tydef));
            break;
        case AST_TYPEDEF_POINTER:
            check_type(static_cast<Ast_Pointer_Type*>(tydef));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Function_Type* ty) {
    ty->inferred_type = Light_Compiler::instance->type_def_type;
}

void Type_Checking::check_type (Ast_Struct_Type* ty) {
    ty->inferred_type = Light_Compiler::instance->type_def_type;
}

void Type_Checking::check_type (Ast_Pointer_Type* ty) {
    ty->inferred_type = Light_Compiler::instance->type_def_type;
}

void Type_Checking::check_type (Ast_Function* func) {
    func->inferred_type = func->type;
}

void Type_Checking::check_type (Ast_Literal* lit) {
    //TODO: smarter inference: use the smallest possible type
    if (!lit->inferred_type) {
        switch (lit->literal_type) {
            case AST_LITERAL_INTEGER: {
				lit->inferred_type = Light_Compiler::instance->type_def_i32;
                break;
            }
            default: break;
        }
    }
}
