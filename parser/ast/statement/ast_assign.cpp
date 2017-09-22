#pragma once

#include <iostream>
#include <vector>

#include "ast_var_def.cpp"

using namespace std;

class ASTAssign : public ASTStatement {
public:
	std::string name = "";
	ASTStatements* stms = nullptr;

	void print (int tabs) {
		cout << "type " << this->name << "\n";
		if (stms != nullptr) stms->print(tabs + 1);
	}
};
