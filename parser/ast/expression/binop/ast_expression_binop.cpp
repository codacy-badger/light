#pragma once

#include "../ast_expression.cpp"

using namespace std;

class ASTExpressionBinop : public ASTExpression {
	public:
		ASTExpression* lhs = NULL;
		ASTExpression* rhs = NULL;
		void print (int tabs);
};

#include "ast_expression_binop_add.cpp"
#include "ast_expression_binop_sub.cpp"
#include "ast_expression_binop_mul.cpp"
#include "ast_expression_binop_div.cpp"

void ASTExpressionBinop::print (int tabs) { /* empty */ }
