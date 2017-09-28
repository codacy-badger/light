#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <map>

#include "ast_type.cpp"

class ASTI32Type : public ASTType {
public:
	ASTI32Type () {
		this->name = "i32";
	}

	bool isPrimitive () {
		return true;
	}
};
