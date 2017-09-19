#pragma once

#include "../ast_node.cpp"

using namespace std;

class ASTExpression : public ASTNode {
	public:
		void print (int tabs);
};

#include "ast_id.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_const.cpp"

void ASTExpression::print (int tabs) { /* empty */ }
