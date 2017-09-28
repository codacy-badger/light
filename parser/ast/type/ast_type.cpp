#pragma once

#include <string>

struct ASTType : ASTNode {
	std::string name;

	bool isPrimitive () {
		return false;
	}
};

struct ASTI32Type : ASTType {
	ASTI32Type () {
		this->name = "i32";
	}

	bool isPrimitive () {
		return true;
	}
};
