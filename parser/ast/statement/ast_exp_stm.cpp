#pragma once

#include "parser/ast/expression/ast_call.cpp"

#include <iostream>

using namespace std;

class ASTExpStatement : public ASTStatement {
public:
	ASTExpression* exp = nullptr;

	void print (int tabs) {
		if (this->exp != nullptr) {
			this->tabs(tabs);
			cout << "EXP ";
			this->exp->print(tabs);
			cout << "\n";
		}
	}
};
