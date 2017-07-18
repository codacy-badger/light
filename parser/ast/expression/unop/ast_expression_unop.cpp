#pragma once


using namespace std;

#include "../ast_expression.cpp"

class ASTExpressionUnop : public ASTExpression {
	public:
		ASTExpression* expression = NULL;
		void print (int tabs);
};

#include "ast_expression_unop_neg.cpp"

void ASTExpressionUnop::print (int tabs) { /* empty */ }
