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
		if (note->arguments) {
			for (int i = 0; i < note->arguments->values.size(); i++) {
				this->fold(&note->arguments->values[i]);
			}
		}
	}
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			auto block = static_cast<Ast_Block*>(stm);
			for (auto stm : block->list) this->fold(stm);
			break;
		}
		case AST_STATEMENT_RETURN: {
			auto ret = static_cast<Ast_Return*>(stm);
			if (ret->exp) this->fold(&ret->exp);
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			auto decl = static_cast<Ast_Declaration*>(stm);
			if (decl->type) this->fold(&decl->type);
			if (decl->expression) this->fold(&decl->expression);
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			auto exp = static_cast<Ast_Expression*>(stm);
			this->fold(&exp);
			break;
		}
	}
}

void Constant_Folding::fold (Ast_Expression** exp) {
	switch ((*exp)->exp_type) {
		case AST_EXPRESSION_FUNCTION: {
			auto func = reinterpret_cast<Ast_Function*>(*exp);
			this->fold(reinterpret_cast<Ast_Type_Definition**>(&func->type));
			if (func->scope) {
				for (auto stm : func->scope->list) this->fold(stm);
			}
			break;
		}
		case AST_EXPRESSION_TYPE_DEFINITION: {
			this->fold(reinterpret_cast<Ast_Type_Definition**>(exp));
			break;
		}
		case AST_EXPRESSION_BINARY: {
			this->fold(reinterpret_cast<Ast_Binary**>(exp));
			break;
		}
		case AST_EXPRESSION_UNARY: {
			this->fold(reinterpret_cast<Ast_Unary**>(exp));
			break;
		}
		case AST_EXPRESSION_CALL: {
			auto call = reinterpret_cast<Ast_Function_Call*>(*exp);
			for (int i = 0; i < call->args->values.size(); i++) {
				this->fold(&call->args->values[i]);
			}
			break;
		}
		case AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS: {
			auto args = reinterpret_cast<Ast_Comma_Separated_Arguments*>(*exp);
			for (int i = 0; i < args->values.size(); i++) {
				this->fold(&args->values[i]);
			}
			break;
		}
		default: break;
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
		ast_copy_location_info(tmp, *binary);
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
				Light_Compiler::inst->warning(*binary, "String literal folding not supported yet!");
				break;
			}
		}
		delete *binary;
		*binary = reinterpret_cast<Ast_Binary*>(tmp);
	}
}

void Constant_Folding::fold (Ast_Unary** unary) {
	this->fold(&(*unary)->exp);
	if ((*unary)->exp->exp_type == AST_EXPRESSION_LITERAL) {
		auto lit = reinterpret_cast<Ast_Literal*>((*unary)->exp);

		auto tmp = new Ast_Literal();
		ast_copy_location_info(tmp, *unary);
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
				Light_Compiler::inst->warning(*unary, "String literal folding not supported yet!");
				break;
			}
		}
		delete *unary;
		*unary = reinterpret_cast<Ast_Unary*>(tmp);
	}
}

void Constant_Folding::fold (Ast_Type_Definition** tydef) {
	switch ((*tydef)->typedef_type) {
		case AST_TYPEDEF_FUNCTION: {
			auto fn_type = reinterpret_cast<Ast_Function_Type*>(*tydef);
			for (auto exp : fn_type->parameter_decls) this->fold(exp);
			break;
		}
		case AST_TYPEDEF_STRUCT: {
			auto _struct = reinterpret_cast<Ast_Struct_Type*>(*tydef);
			for (auto decl : _struct->attributes) this->fold(decl);
			break;
		}
		default: break;
	}
}
