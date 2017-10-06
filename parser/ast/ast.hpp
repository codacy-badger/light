#pragma once

#include "lexer/lexer.cpp"

struct ASTNode {
	long line, col;
	const char* filename;

	~ASTNode () {}

	void print () {
		cout << typeid(*this).name() << " ";
		cout << filename << "(" << line;
		cout << ", " << col << ")\n";
	}
};

#include "ast_statement.cpp"
#include "expression/ast_expression.cpp"
#include "ast_context.cpp"
#include "ast_printer.cpp"
