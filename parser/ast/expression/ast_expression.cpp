#pragma once

struct ASTType;
struct ASTContext;

struct ASTExpression : ASTStatement {
	virtual ~ASTExpression() {}
	virtual bool isConstant() = 0;
	virtual ASTType* getType(ASTContext* context) = 0;
};

#include "ast_type.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_value.cpp"

struct ASTFunction : ASTExpression {
	std::string name;
	ASTFnType* type = nullptr;
	ASTStatement* stms = nullptr;

	bool isConstant() { return false; }
	ASTType* getType(ASTContext* context) {
		return this->type;
	}
};
