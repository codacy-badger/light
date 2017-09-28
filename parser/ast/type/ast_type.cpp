#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <map>

#include "../ast_node.cpp"

class ASTType : public ASTNode{
public:
	std::string name;

	bool isPrimitive () {
		return false;
	}
};

#include "ast_i32_type.cpp"
