#pragma once

#include <string>
#include <vector>

struct ASTVarDef;

struct ASTType : ASTStatement {
	std::string name;
	ASTStatements* stms = nullptr;

	bool isPrimitive () {
		return false;
	}
};

struct ASTFnType : ASTType {
	std::vector<ASTVarDef*> params;
	ASTType* retType = nullptr;
};

struct ASTI32Type : ASTType {
	ASTI32Type () {
		this->name = "i32";
	}

	bool isPrimitive () {
		return true;
	}
};
