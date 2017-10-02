#pragma once

#include <vector>
#include <string>

struct ASTType;
struct ASTFunction;
struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};

struct ASTScope : ASTStatement {
	std::vector<ASTType*> types;
	std::vector<ASTFunction*> functions;
	std::vector<ASTStatement*> list;
};
