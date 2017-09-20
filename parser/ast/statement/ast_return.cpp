#pragma once

#include "parser/ast/ast_type.cpp"
#include "parser/ast/expression/ast_expression.cpp"

#include <iostream>

using namespace std;

class ASTReturn : public ASTStatement {
public:
	ASTExpression* expression = nullptr;

	void print (int tabs) {
		this->tabs(tabs);
		cout << "RETURN ";
		if (this->expression == nullptr) cout << "void";
		else this->expression->print(tabs);
		cout << endl;
	}
};
