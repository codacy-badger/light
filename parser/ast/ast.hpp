#pragma once

#include "lexer/lexer.cpp"

struct ASTNode {
	long line, col;
	const char* filename;

	~ASTNode () {}

	void print () {
		cout << "\tat " << filename << "(" << line;
		cout << ", " << col << ")\n";
	}
};

#include "ast_statement.cpp"
#include "expression/ast_expression.cpp"
