#pragma once


using namespace std;

#include "../ast_node.cpp"

class ASTExpression : public ASTNode {
	public:
		void print (int tabs);
};

#include "binop/ast_expression_binop.cpp"
#include "unop/ast_expression_unop.cpp"
#include "ast_def_function.cpp"
#include "ast_def_type.cpp"
#include "ast_variable.cpp"
#include "ast_number.cpp"
#include "ast_string.cpp"

void ASTExpression::print (int tabs) {
	if(ASTExpressionUnop* v = dynamic_cast<ASTExpressionUnop*>(this)) {
		v->print(tabs);
	} else if (ASTExpressionBinop* v = dynamic_cast<ASTExpressionBinop*>(this)) {
		v->print(tabs);
	} else if (ASTVariable* v = dynamic_cast<ASTVariable*>(this)) {
		v->print(tabs);
	} else if (ASTNumber* v = dynamic_cast<ASTNumber*>(this)) {
		v->print(tabs);
	} else if (ASTDefFunction* v = dynamic_cast<ASTDefFunction*>(this)) {
		v->print(tabs);
	} else if (ASTDefType* v = dynamic_cast<ASTDefType*>(this)) {
		v->print(tabs);
	}
}
