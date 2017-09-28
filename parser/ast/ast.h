#pragma once

#include "lexer/lexer.cpp"

struct ASTNode {
	static int _count;

	unsigned int lineBegin = 0, colBegin = 0;
	unsigned int lineEnd = 0, colEnd = 0;

	void setBegin (Lexer* lexer) {
		this->lineBegin = lexer->buffer->line;
		this->colBegin = lexer->buffer->col;
	}

	void setEnd (Lexer* lexer) {
		this->lineEnd = lexer->buffer->line;
		this->colEnd = lexer->buffer->col;
	}
};

#include "statement/ast_statement.cpp"
#include "type/ast_type.cpp"
#include "ast_context.cpp"
#include "expression/ast_expression.cpp"
#include "ast_printer.cpp"
