#pragma once

#include "parser/ast/expression/ast_call.cpp"

#include <iostream>

using namespace std;

class ASTCallStatement : public ASTStatement {
public:
	ASTCall* call = nullptr;

	void print (int tabs) {
		if (this->call != nullptr) {
			this->tabs(tabs);
			this->call->print(tabs);
			cout << "\n";
		}
	}
};
