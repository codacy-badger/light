#pragma once

#include <vector>
#include <string>

struct ASTType;
struct ASTFunction;
struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}
};

struct ASTStatements : ASTStatement {
	std::vector<ASTType*> types;
	std::vector<ASTFunction*> functions;
	std::vector<ASTStatement*> list;
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};
