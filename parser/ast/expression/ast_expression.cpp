#pragma once

struct ASTType;
struct ASTContext;

struct ASTExpression : ASTStatement {
	virtual ~ASTExpression() {}
	virtual bool isConstant() = 0;
	virtual ASTType* getType() = 0;
};

#include "ast_type.cpp"

struct ASTFunction : ASTExpression {
	std::string name;
	ASTFnType* type = nullptr;
	ASTStatement* stm = nullptr;

	bool isConstant() { return false; }
	ASTType* getType() { return this->type; }
};

#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_value.cpp"
