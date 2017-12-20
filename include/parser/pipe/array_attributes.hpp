#pragma once

#include "parser/pipes.hpp"

struct Array_Attributes : Pipe {
	void on_statement(Ast_Statement* stm) {
		this->replace(stm);
		this->to_next(stm);
	}

	void replace (Ast_Statement* stm) {
		switch (stm->stm_type) {
			case AST_STATEMENT_BLOCK: {
				auto block = static_cast<Ast_Block*>(stm);
				for (auto _stm : block->list) {
					this->replace(_stm);
				}
				break;
			}
			case AST_STATEMENT_IF: {
				auto _if = static_cast<Ast_If*>(stm);
				this->replace(&_if->condition);
				this->replace(_if->then_statement);
				if (_if->else_statement) this->replace(_if->else_statement);
				break;
			}
			case AST_STATEMENT_WHILE: {
				auto _while = static_cast<Ast_While*>(stm);
				this->replace(&_while->condition);
				this->replace(_while->statement);
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				auto decl = static_cast<Ast_Declaration*>(stm);
				if (decl->expression) this->replace(&decl->expression);
				break;
			}
			case AST_STATEMENT_RETURN: {
				auto ret = static_cast<Ast_Return*>(stm);
				this->replace(&ret->exp);
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				auto exp = static_cast<Ast_Expression*>(stm);
				this->replace(&exp);
				break;
			}
			default: break;
		}
	}

	void replace (Ast_Expression** exp) {
		switch ((*exp)->exp_type) {
			case AST_EXPRESSION_BINARY: {
				auto binary = static_cast<Ast_Binary*>(*exp);
				if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
					if (binary->lhs->inferred_type->typedef_type == AST_TYPEDEF_ARRAY) {
						auto arr_type = static_cast<Ast_Array_Type*>(binary->lhs->inferred_type);
						if (arr_type->kind == AST_ARRAY_KIND_STATIC) {
							auto lit = ast_make_literal(arr_type->length());
							lit->inferred_type = Light_Compiler::inst->type_def_u64;
							delete *exp;
							(*exp) = lit;
						}
					}
				} else {
					this->replace(&binary->lhs);
					this->replace(&binary->rhs);
				}
				break;
			}
			case AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS: {
				auto args = static_cast<Ast_Comma_Separated_Arguments*>(*exp);
				for (int i = 0; i < args->values.size(); i++) {
					this->replace(&args->values[i]);
				}
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				auto func = static_cast<Ast_Function*>(*exp);
				this->replace(func->scope);
				break;
			}
			case AST_EXPRESSION_UNARY: {
				auto unary = static_cast<Ast_Unary*>(*exp);
				this->replace(&unary->exp);
				break;
			}
			case AST_EXPRESSION_CALL: {
				auto call = static_cast<Ast_Function_Call*>(*exp);
				if (call->args) {
					for (int i = 0; i < call->args->values.size(); i++) {
						this->replace(&call->args->values[i]);
					}
				}
				break;
			}
			case AST_EXPRESSION_CAST: {
				auto cast = static_cast<Ast_Cast*>(*exp);
				this->replace(&cast->value);
				break;
			}
			default: break;
		}
	}
};
