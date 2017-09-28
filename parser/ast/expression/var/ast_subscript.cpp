#pragma once

#include <iostream>

#include "../ast_expression.cpp"

using namespace std;

class ASTSubscript : public ASTVariable {
	public:
		ASTExpression* expression = NULL;
		ASTExpression* index = NULL;

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
