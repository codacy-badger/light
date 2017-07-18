#pragma once


using namespace std;

#include "ast_expression.cpp"

class ASTVariable : public ASTExpression {
	public:
		string name;

		void print (int tabs) {
			cout << "[" << this->name << "]";
		}
};


