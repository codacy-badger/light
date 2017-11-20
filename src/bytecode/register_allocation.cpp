#include "bytecode/register_allocation.hpp"

#include "compiler.hpp"

void Register_Allocation::on_statement(Ast_Statement* stm) {
    this->assign(stm);
    this->to_next(stm);
}

void Register_Allocation::assign (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK: {
            this->assign(static_cast<Ast_Block*>(stm));
            return;
        }
        case AST_STATEMENT_RETURN: {
            this->assign(static_cast<Ast_Return*>(stm));
            return;
        }
        case AST_STATEMENT_DECLARATION: {
            this->assign(static_cast<Ast_Declaration*>(stm));
            return;
        }
        case AST_STATEMENT_EXPRESSION: {
            this->assign(static_cast<Ast_Expression*>(stm));
            return;
        }
    }
}

void Register_Allocation::assign (Ast_Block* block) {
    for (auto stm : block->list) {
        this->assign(stm);
    }
}

void Register_Allocation::assign (Ast_Return* ret) {
    if (ret->exp) this->assign(ret->exp);
}

void Register_Allocation::assign (Ast_Declaration* decl) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
        if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(decl->expression);
            if (!func->foreign_module_name) {
                for (auto decl : func->type->parameter_decls) {
                    this->assign(decl);
                }
    			this->current_register = 0;
    			this->assign(func->scope);
            }
		}
    } else {
        auto ty_decl = static_cast<Ast_Type_Definition*>(decl->type);
        if (decl->scope->is_global()) {
            Light_Compiler::inst->error_stop(decl, "Global variables not yet supported!");
		} else {
			if (decl->expression) this->assign(decl->expression);
		}
    }
}

void Register_Allocation::assign (Ast_Function* fn) {
}

void Register_Allocation::assign (Ast_Expression* exp) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL: {
            this->assign(static_cast<Ast_Literal*>(exp));
            return;
        }
        case AST_EXPRESSION_IDENT: {
            this->assign(static_cast<Ast_Ident*>(exp));
            return;
        }
        case AST_EXPRESSION_UNARY: {
            this->assign(static_cast<Ast_Unary*>(exp));
            return;
        }
        case AST_EXPRESSION_BINARY: {
            this->assign(static_cast<Ast_Binary*>(exp));
            return;
        }
        case AST_EXPRESSION_CALL: {
            this->assign(static_cast<Ast_Function_Call*>(exp));
            return;
        }
        case AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS: {
            this->assign(static_cast<Ast_Comma_Separated_Arguments*>(exp));
            return;
        }
    }
}

void Register_Allocation::assign (Ast_Literal* lit) {
    lit->register_index = this->current_register++;
}

void Register_Allocation::assign (Ast_Ident* ident) {
    ident->register_index = this->current_register++;
}

void Register_Allocation::assign (Ast_Unary* unop) {
    this->assign(unop->exp);
    unop->register_index = unop->exp->register_index;
}

void Register_Allocation::assign (Ast_Binary* binop) {
    this->assign(binop->lhs);
    this->assign(binop->rhs);
    binop->register_index = binop->lhs->register_index;
}

void Register_Allocation::assign (Ast_Function_Call* call) {
	auto _tmp = this->current_register;
    this->current_register = 1;
    for (auto exp : call->parameters) {
        this->assign(exp);
    }
	this->current_register = _tmp + 1;
    call->register_index = 0;
}

void Register_Allocation::assign (Ast_Comma_Separated_Arguments* args) {
    for (auto exp : args->args) this->assign(exp);
}
