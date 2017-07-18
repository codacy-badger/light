#pragma once


using namespace std;

#include "ast_expression.cpp"

class ASTNumber : public ASTExpression {
	public:
		string value;

		void print (int tabs) {
			cout << this->value;
		}
};


