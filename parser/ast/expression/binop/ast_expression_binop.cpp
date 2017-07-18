#pragma once


using namespace std;

#include "../ast_expression.cpp"

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

void ASTExpressionBinop::print (int tabs) {
	if(ASTExpressionBinopAdd* v = dynamic_cast<ASTExpressionBinopAdd*>(this)) {
		v->print(tabs);
	} else if(ASTExpressionBinopSub* v = dynamic_cast<ASTExpressionBinopSub*>(this)) {
		v->print(tabs);
	} else if(ASTExpressionBinopMul* v = dynamic_cast<ASTExpressionBinopMul*>(this)) {
		v->print(tabs);
	} else if(ASTExpressionBinopDiv* v = dynamic_cast<ASTExpressionBinopDiv*>(this)) {
		v->print(tabs);
	}
}
