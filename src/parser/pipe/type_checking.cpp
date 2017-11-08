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
	if (decl->expression) check_type(decl->expression);

	if (decl->type) {
		check_type(decl->type);
		if (decl->type->exp_type == AST_EXPRESSION_IDENT) {
			auto ident = static_cast<Ast_Ident*>(decl->type);
			delete decl->type;
			decl->type = ident->declaration->expression;
		}
	}

	if (!decl->type && !decl->expression) {
		Light_Compiler::instance->error_stop(decl, "Cannot infer type without an expression!");
	} else if (!decl->type) {
		decl->type = decl->expression->inferred_type;
	}

	if (decl->expression && (decl->decl_flags & DECL_FLAG_CONSTANT)) {
		// TODO: ensure expression is constant
	}

	if (decl->type->inferred_type != Light_Compiler::instance->type_def_type) {
		Light_Compiler::instance->error_stop(decl->type, "Expression is not a type!");
	}
}

void Type_Checking::check_type (Ast_Return* ret) {
	if (ret->exp) check_type(ret->exp);

	auto fn = ret->block->find_function();
	if (!fn) {
		Light_Compiler::instance->error_stop(ret, "Return statement must be inside a function!");
	} else if (ret->exp) {
		if (fn->type->return_type == Light_Compiler::instance->type_def_void)
			Light_Compiler::instance->error_stop(ret, "Return statment has expression, but function returns void!");
		else if (ret->exp->inferred_type != fn->type->return_type) {
			Light_Compiler::instance->error_stop(ret, "Type mismatch, return expression is '---', but function expects '---'!");
		}
	} else {
		if (fn->type->return_type != Light_Compiler::instance->type_def_void)
			Light_Compiler::instance->error_stop(ret, "Return statment has no expression, but function returns '---'!");
	}
}

void Type_Checking::check_type (Ast_Expression* exp) {
    switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION:
            check_type(static_cast<Ast_Type_Definition*>(exp));
            break;
        case AST_EXPRESSION_FUNCTION:
            check_type(static_cast<Ast_Function*>(exp));
            break;
        case AST_EXPRESSION_BINARY:
            check_type(static_cast<Ast_Binary*>(exp));
            break;
        case AST_EXPRESSION_UNARY:
            check_type(static_cast<Ast_Unary*>(exp));
            break;
        case AST_EXPRESSION_IDENT:
            check_type(static_cast<Ast_Ident*>(exp));
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

	check_type(ty->return_type);
	if (ty->return_type->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(ty->return_type);
		delete ty->return_type;
		ty->return_type = ident->declaration->expression;
	}

	for (int i = 0; i < ty->parameter_types.size(); i++) {
		check_type(ty->parameter_types[i]);
		if (ty->parameter_types[i]->exp_type == AST_EXPRESSION_IDENT) {
			auto ident = static_cast<Ast_Ident*>(ty->parameter_types[i]);
			delete ty->parameter_types[i];
			ty->parameter_types[i] = ident->declaration->expression;
		}
	}
}

void Type_Checking::check_type (Ast_Struct_Type* ty) {
    ty->inferred_type = Light_Compiler::instance->type_def_type;
	for (auto decl : ty->attributes) {
		check_type(decl);
	}
}

void Type_Checking::check_type (Ast_Pointer_Type* ty) {
    ty->inferred_type = Light_Compiler::instance->type_def_type;
	check_type(ty->base);
	if (ty->base->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(ty->base);
		delete ty->base;
		ty->base = ident->declaration->expression;
	}
}

void Type_Checking::check_type (Ast_Function* func) {
    func->inferred_type = func->type;
	check_type(func->type);
	check_type(func->scope);
}

void Type_Checking::check_type (Ast_Binary* binop) {
	check_type(binop->lhs);
	check_type(binop->rhs);
	if (binop->lhs->inferred_type == binop->rhs->inferred_type) {
		binop->inferred_type = binop->lhs->inferred_type;
	} else {
		Light_Compiler::instance->error_stop(binop, "Type mismatch on binary expression");
	}
}

void Type_Checking::check_type (Ast_Unary* unop) {
	check_type(unop->exp);
	unop->inferred_type = unop->exp->inferred_type;
}

void Type_Checking::check_type (Ast_Ident* ident) {
	if (ident->declaration) {
		ident->inferred_type = static_cast<Ast_Type_Definition*>(ident->declaration->type);
	} else {
		Light_Compiler::instance->error_stop(ident, "Symbol not found...");
	}
}

void Type_Checking::check_type (Ast_Literal* lit) {
    //TODO: smarter inference: use the smallest possible type
    if (!lit->inferred_type) {
        switch (lit->literal_type) {
            case AST_LITERAL_SIGNED_INT: {
				lit->inferred_type = Light_Compiler::instance->type_def_i32;
                break;
            }
            default: break;
        }
    }
}
