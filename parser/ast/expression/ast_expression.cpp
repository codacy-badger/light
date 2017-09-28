#pragma once

#include "../ast_node.cpp"

class ASTExpression : public ASTNode {
public:
	void print (int tabs) = 0;
	virtual bool isConstant() = 0;
};

#include "var/ast_variable.cpp"
#include "ast_const.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_call.cpp"
