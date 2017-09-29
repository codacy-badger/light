#pragma once

struct ASTExpression : ASTStatement {
	virtual ~ASTExpression() {}
	virtual bool isConstant() = 0;
	virtual ASTType* getType(ASTContext* context) = 0;
};

#include "var/ast_variable.cpp"
#include "ast_const.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"

struct ASTCall : ASTExpression {
	ASTVariable* var = nullptr;
	std::vector<ASTExpression*> params;

	bool isConstant () {
		return false;
	}

	ASTType* getType(ASTContext* context) {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
