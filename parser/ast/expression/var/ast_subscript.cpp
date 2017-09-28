#pragma once

#include <iostream>

#include "../ast_expression.cpp"

using namespace std;

class ASTSubscript : public ASTVariable {
	public:
		ASTExpression* expression = NULL;
		ASTExpression* index = NULL;

		void print (int tabs) {
			this->expression->print(tabs);
			cout << "[";
			this->index->print(tabs);
			cout << "]";
		}

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
