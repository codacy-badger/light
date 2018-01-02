#pragma once

#include "parser/pipes.hpp"

template<typename T>
T binary_fold (Ast_Binary_Type binary_op, T a, T b) {
	switch (binary_op) {
		case AST_BINARY_LOGICAL_AND: 	return a && b;
		case AST_BINARY_LOGICAL_OR: 	return a || b;

		case AST_BINARY_ADD: return a + b;
		case AST_BINARY_SUB: return a - b;
		case AST_BINARY_MUL: return a * b;
		case AST_BINARY_DIV: return a / b;

		case AST_BINARY_EQ: 	return a == b;
		case AST_BINARY_NEQ: 	return a != b;
		case AST_BINARY_LT: 	return a < b;
		case AST_BINARY_LTE: 	return a <= b;
		case AST_BINARY_GT: 	return a > b;
		case AST_BINARY_GTE: 	return a >= b;
		default: return 0;
	}
}

template<typename T>
T binary_fold_logic (Ast_Binary_Type binary_op, T a, T b) {
	switch (binary_op) {
		case AST_BINARY_BITWISE_AND: 			return a & b;
		case AST_BINARY_BITWISE_OR: 			return a | b;
		case AST_BINARY_BITWISE_XOR: 			return a ^ b;
		case AST_BINARY_BITWISE_RIGHT_SHIFT: 	return a >> b;
		case AST_BINARY_BITWISE_LEFT_SHIFT: 	return a << b;
		case AST_BINARY_REM: 					return a % b;
		default: 								return binary_fold(binary_op, a, b);
	}
}

template<typename T>
T unary_fold (Ast_Unary_Type unary_op, T a) {
	switch (unary_op) {
		case AST_UNARY_NEGATE: 	return -a;
		case AST_UNARY_NOT: 	return !a;
		default: return 0;
	}
}

struct Constant_Folding : Pipe {

	void handle (Ast_Unary** unary_ptr) {
		Pipe::handle(unary_ptr);
		auto unary = (*unary_ptr);

		if (unary->exp->exp_type == AST_EXPRESSION_LITERAL) {
			auto lit = reinterpret_cast<Ast_Literal*>(unary->exp);

			Ast_Unary_Type unop = unary->unary_op;
			switch (lit->literal_type) {
				case AST_LITERAL_UNSIGNED_INT: {
					lit->int_value = unary_fold(unop, lit->int_value);
					if (unop == AST_UNARY_NEGATE)
						lit->literal_type = AST_LITERAL_SIGNED_INT;
					break;
				}
				case AST_LITERAL_SIGNED_INT: {
					lit->int_value = unary_fold(unop, lit->int_value);
					break;
				}
				case AST_LITERAL_DECIMAL: {
					lit->decimal_value = unary_fold(unop, lit->decimal_value);
					break;
				}
				case AST_LITERAL_STRING: {
					report_warning(&unary->location, "String literal folding not supported yet!");
					break;
				}
			}
			delete *unary_ptr;
			*unary_ptr = reinterpret_cast<Ast_Unary*>(lit);
		}
	}

	void handle (Ast_Binary** binary_ptr) {
		Pipe::handle(binary_ptr);
		auto binary = (*binary_ptr);

		if (binary->lhs->exp_type == AST_EXPRESSION_LITERAL
			&& binary->rhs->exp_type == AST_EXPRESSION_LITERAL) {
			auto litL = reinterpret_cast<Ast_Literal*>(binary->lhs);
			auto litR = reinterpret_cast<Ast_Literal*>(binary->rhs);

			auto tmp = new Ast_Literal();
			tmp->location = binary->location;
			tmp->literal_type = litL->literal_type;
			Ast_Binary_Type binop = binary->binary_op;
			switch (litL->literal_type) {
				case AST_LITERAL_UNSIGNED_INT: {
					tmp->uint_value = binary_fold_logic(binop, litL->uint_value, litR->uint_value);
					break;
				}
				case AST_LITERAL_SIGNED_INT: {
					tmp->int_value = binary_fold_logic(binop, litL->int_value, litR->int_value);
					break;
				}
				case AST_LITERAL_DECIMAL: {
					tmp->decimal_value = binary_fold(binop, litL->decimal_value, litR->decimal_value);
					break;
				}
				case AST_LITERAL_STRING: {
					report_warning(&binary->location, "String literal folding not supported yet!");
					break;
				}
			}
			delete *binary_ptr;
			*binary_ptr = reinterpret_cast<Ast_Binary*>(tmp);
		}
	}
};
