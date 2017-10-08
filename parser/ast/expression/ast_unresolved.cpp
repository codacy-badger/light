#pragma once

#include <string>

struct ASTUnresolved {
	std::string name;

	ASTUnresolved (std::string name = "") {
		this->name = name;
	}
};

struct ASTUnresolvedExp : ASTUnresolved, ASTExpression {
	std::string name;

	ASTUnresolvedExp (std::string name = "") : ASTUnresolved(name)
	{ /* empty */ }

	virtual bool isConstant() { return false; }
	virtual ASTType* getType() { return nullptr; }
};

struct ASTUnresolvedTy : ASTUnresolved, ASTType {

	ASTUnresolvedTy (std::string name = "") : ASTUnresolved(name)
	{ /* empty */ }

	virtual bool isPrimitive () { return false; }
};

struct ASTUnresolvedFn : ASTUnresolved, ASTFunction {

	ASTUnresolvedFn (std::string name = "") : ASTUnresolved(name)
	{ /* empty */ }
};
