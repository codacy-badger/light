#pragma once

#include <iostream>

using namespace std;

class ASTVariable : public ASTExpression {
public:
	void print (int tabs) = 0;

	bool isConstant () {
		// TODO: not necessarily
		return false;
	}

	ASTType* getType(ParserContext* context) = 0;
};

#include "ast_id.cpp"
#include "ast_attr.cpp"
#include "ast_subscript.cpp"
