#pragma once


using namespace std;

#include "ast_expression.cpp"

class ASTString : public ASTExpression {
	public:
		string value;

		void print (int tabs) {
			cout << "\"" << this->value << "\"";
		}
};
