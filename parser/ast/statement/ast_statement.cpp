#pragma once

#include <vector>
#include <string>

struct ASTType;
struct ASTFnType;
struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}
};

struct ASTStatements : ASTStatement {
	std::vector<ASTStatement*> list;
};

struct ASTVarDef : ASTStatement {
	ASTType* type;
	std::string name = "";
	ASTExpression* expression = nullptr;
};

struct ASTFunction : ASTStatement {
	std::string name;
	ASTFnType* type = nullptr;
	ASTStatement* stms = nullptr;
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};
