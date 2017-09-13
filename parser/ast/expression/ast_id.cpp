#pragma once

#include <iostream>

#include "ast_id.cpp"

using namespace std;

class ASTId : public ASTExpression {
	public:
		string name;

		void print (int tabs) {
			cout << "[" << this->name << "]";
		}
};
