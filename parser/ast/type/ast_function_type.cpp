#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <map>

#include "ast_type.cpp"

class ASTFunctionType : public ASTType {
public:
	vector<ASTFnParam*> params;
	ASTType* retType = nullptr;

	void print (int tabs) {
		if (this->name == "") std::cout  << "[?]";
		else std::cout << "[" << this->name << "]";
	}
};
