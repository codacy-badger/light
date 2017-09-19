#pragma once

#include <iostream>

#include "ast_expression.cpp"

using namespace std;

class ASTNumber : public ASTExpression {
public:
	string value;

	void print (int tabs) {
		cout << this->value;
	}
};
