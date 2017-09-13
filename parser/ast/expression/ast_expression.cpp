#pragma once

#include "../ast_node.cpp"

using namespace std;

class ASTExpression : public ASTNode {
	public:
		void print (int tabs);
};

#include "ast_id.cpp"
#include "binop/ast_expression_binop.cpp"
#include "unop/ast_expression_unop.cpp"
#include "ast_number.cpp"
#include "ast_string.cpp"

void ASTExpression::print (int tabs) { /* empty */ }
