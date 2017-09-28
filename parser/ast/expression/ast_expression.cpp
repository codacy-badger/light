#pragma once

#include "../ast_node.cpp"
#include "../../parser_context.cpp"

class ASTExpression : public ASTNode {
public:
	virtual ~ASTExpression() {}
	virtual bool isConstant() = 0;
	virtual ASTType* getType(ParserContext* context) = 0;
};

#include "var/ast_variable.cpp"
#include "ast_const.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_call.cpp"
