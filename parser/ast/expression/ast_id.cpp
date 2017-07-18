#pragma once


using namespace std;

#include "ast_id.cpp"

class ASTId : public ASTExpression {
	public:
		string name;

		void print (int tabs) {
			cout << "[" << this->name << "]";
		}
};
