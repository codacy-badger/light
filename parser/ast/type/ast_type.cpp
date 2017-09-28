#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <map>

#include "../ast_node.cpp"

class ASTType : public ASTNode{
public:
	std::string name;

	void print (int tabs) {
		if (this->name == "") std::cout  << "[?]";
		else std::cout << this->name;
	}

	bool isPrimitive () {
		return false;
	}
};

#include "ast_i32_type.cpp"
