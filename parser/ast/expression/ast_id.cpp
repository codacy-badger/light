#pragma once

#include <iostream>

using namespace std;

class ASTId : public ASTExpression {
	public:
		string name;

		void print (int tabs) {
			cout << "[" << this->name << "]";
		}
};
