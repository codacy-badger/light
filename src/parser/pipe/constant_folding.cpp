#include "parser/pipe/constant_folding.hpp"

#include <stdio.h>

#include "compiler.hpp"

template<typename T>
T binary_fold (Ast_Binary_Type binary_op, T a, T b) {
	switch (binary_op) {
		case AST_BINARY_ADD: return a + b;
		case AST_BINARY_SUB: return a - b;
		case AST_BINARY_MUL: return a * b;
		case AST_BINARY_DIV: return a / b;
		default: return 0;
	}
}

template<typename T>
T unary_fold (Ast_Unary_Type unary_op, T a) {
	switch (unary_op) {
		case AST_UNARY_NEGATE: return -a;
		case AST_UNARY_NOT: return !a;
		default: return 0;
	}
}

void Constant_Folding::on_statement(Ast_Statement* stm) {
	this->fold(stm);
	this->to_next(stm);
}

void Constant_Folding::fold (Ast_Statement* stm) {
	for (auto note : stm->notes) {
		if (note->arguments) this->fold(&note->arguments);
	}
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			this->fold(static_cast<Ast_Block*>(stm));
			break;
		}
		case AST_STATEMENT_RETURN: {
			this->fold(static_cast<Ast_Return*>(stm));
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			this->fold(static_cast<Ast_Declaration*>(stm));
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			auto exp = static_cast<Ast_Expression*>(stm);
			this->fold(&exp);
			break;
		}
	}
}

void Constant_Folding::fold (Ast_Block* block) {
	for (auto stm : block->list)
		this->fold(stm);
}

void Constant_Folding::fold (Ast_Return* ret) {
	if (ret->exp) this->fold(&ret->exp);
}

void Constant_Folding::fold (Ast_Declaration* decl) {
	if (decl->type) this->fold(&decl->type);
	if (decl->expression) this->fold(&decl->expression);
}

void Constant_Folding::fold (Ast_Expression** exp) {
	switch ((*exp)->exp_type) {
		case AST_EXPRESSION_FUNCTION: {
			this->fold(reinterpret_cast<Ast_Function**>(exp));
			return;
		}
		case AST_EXPRESSION_CALL: {
			this->fold(reinterpret_cast<Ast_Function_Call**>(exp));
			return;
		}
		case AST_EXPRESSION_TYPE_DEFINITION: {
			this->fold(reinterpret_cast<Ast_Type_Definition**>(exp));
			return;
		}
		case AST_EXPRESSION_BINARY: {
			this->fold(reinterpret_cast<Ast_Binary**>(exp));
			return;
		}
		case AST_EXPRESSION_UNARY: {
			this->fold(reinterpret_cast<Ast_Unary**>(exp));
			return;
		}
		default: return;
	}
}

void Constant_Folding::fold (Ast_Binary** binary) {
	this->fold(&(*binary)->lhs);
	this->fold(&(*binary)->rhs);
	if ((*binary)->lhs->exp_type == AST_EXPRESSION_LITERAL
		&& (*binary)->rhs->exp_type == AST_EXPRESSION_LITERAL) {
		auto litL = reinterpret_cast<Ast_Literal*>((*binary)->lhs);
		auto litR = reinterpret_cast<Ast_Literal*>((*binary)->rhs);

		auto tmp = new Ast_Literal();
		tmp->literal_type = litL->literal_type;
		Ast_Binary_Type binop = (*binary)->binary_op;
		switch (litL->literal_type) {
			case AST_LITERAL_UNSIGNED_INT: {
				tmp->uint_value = binary_fold(binop, litL->uint_value, litR->uint_value);
				break;
			}
			case AST_LITERAL_SIGNED_INT: {
				tmp->int_value = binary_fold(binop, litL->int_value, litR->int_value);
				break;
			}
			case AST_LITERAL_DECIMAL: {
				tmp->decimal_value = binary_fold(binop, litL->decimal_value, litR->decimal_value);
				break;
			}
			case AST_LITERAL_STRING: {
				Light_Compiler::inst->error_stop(*binary, "String literal folding not supported yet!");
				break;
			}
		}
		//TODO: copy location info to the new literal
		delete *binary;
		*binary = reinterpret_cast<Ast_Binary*>(tmp);
	}
}

void Constant_Folding::fold (Ast_Unary** unary) {
	this->fold(&(*unary)->exp);
	if ((*unary)->exp->exp_type == AST_EXPRESSION_LITERAL) {
		auto lit = reinterpret_cast<Ast_Literal*>((*unary)->exp);

		auto tmp = new Ast_Literal();
		tmp->literal_type = lit->literal_type;
		Ast_Unary_Type unop = (*unary)->unary_op;
		switch (lit->literal_type) {
			case AST_LITERAL_UNSIGNED_INT: {
				tmp->uint_value = unary_fold(unop, lit->uint_value);
				break;
			}
			case AST_LITERAL_SIGNED_INT: {
				tmp->int_value = unary_fold(unop, lit->int_value);
				break;
			}
			case AST_LITERAL_DECIMAL: {
				tmp->decimal_value = unary_fold(unop, lit->decimal_value);
				break;
			}
			case AST_LITERAL_STRING: {
				Light_Compiler::inst->error_stop(*unary, "String literal folding not supported yet!");
				break;
			}
		}
		//TODO: copy location info to the new literal
		delete *unary;
		*unary = reinterpret_cast<Ast_Unary*>(tmp);
	}
}

void Constant_Folding::fold (Ast_Function** fn) {
	this->fold((*fn)->type);
	this->fold((*fn)->scope);
}

void Constant_Folding::fold (Ast_Function_Call** call) {
	for (int i = 0; i < (*call)->parameters.size(); i++) {
		this->fold(&(*call)->parameters[i]);
	}
}

void Constant_Folding::fold (Ast_Comma_Separated_Arguments** args) {
	for (int i = 0; i < (*args)->args.size(); i++) {
		this->fold(&(*args)->args[i]);
	}
}

void Constant_Folding::fold (Ast_Type_Definition** tydef) {
	switch ((*tydef)->typedef_type) {
		case AST_TYPEDEF_FUNCTION: {
			this->fold(reinterpret_cast<Ast_Function_Type**>(tydef));
			break;
		}
		case AST_TYPEDEF_STRUCT: {
			this->fold(reinterpret_cast<Ast_Struct_Type**>(tydef));
			break;
		}
		default: break;
	}
}

void Constant_Folding::fold (Ast_Struct_Type** _struct) {
	for (auto decl : (*_struct)->attributes) {
		this->fold(decl);
	}
}

void Constant_Folding::fold (Ast_Function_Type** fn_type) {
	for (auto exp : (*fn_type)->parameter_decls) {
		this->fold(exp);
	}
}
