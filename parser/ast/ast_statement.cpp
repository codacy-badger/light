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

#include "ast_block.cpp"
