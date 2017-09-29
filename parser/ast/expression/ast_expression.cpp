#pragma once

struct ASTType;
struct ASTContext;

struct ASTExpression : ASTStatement {
	virtual ~ASTExpression() {}
	virtual bool isConstant() = 0;
	virtual ASTType* getType(ASTContext* context) = 0;
};

#include "ast_type.cpp"
#include "ast_const.cpp"
#include "ast_binop.cpp"
#include "ast_unop.cpp"
#include "ast_memory.cpp"

struct ASTFunction : ASTExpression {
	std::string name;
	ASTFnType* type = nullptr;
	ASTStatement* stms = nullptr;

	bool isConstant() { return false; }
	ASTType* getType(ASTContext* context) {
		return this->type;
	}
};

struct ASTCall : ASTExpression {
	ASTExpression* var = nullptr;
	std::vector<ASTExpression*> params;

	bool isConstant () {
		return false;
	}

	ASTType* getType(ASTContext* context) {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
