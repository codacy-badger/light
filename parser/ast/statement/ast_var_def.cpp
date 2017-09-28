#pragma once

#include "parser/ast/type/ast_type.cpp"
#include "parser/ast/expression/ast_expression.cpp"

#include <iostream>

using namespace std;

class ASTVarDef : public ASTStatement {
	public:
		ASTType* type;
		string name = "";

		ASTExpression* expression = NULL;
};
