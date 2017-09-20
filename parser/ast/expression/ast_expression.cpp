#pragma once

#include "../ast_node.cpp"

class ASTExpression : public ASTNode {
	public:
		void print (int tabs) = 0;
};

#include "ast_id.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_const.cpp"
#include "ast_subscript.cpp"
