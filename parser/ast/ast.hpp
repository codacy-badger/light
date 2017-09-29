#pragma once

#include "lexer/lexer.cpp"

struct ASTNode {
	~ASTNode () {}
};

#include "ast_statement.cpp"
#include "expression/ast_expression.cpp"
#include "ast_context.cpp"
#include "ast_printer.cpp"
