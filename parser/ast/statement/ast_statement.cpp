#pragma once

#include <vector>
#include <string>

struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}
};

struct ASTStatements : ASTStatement {
	std::vector<ASTStatement*> list;
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};
