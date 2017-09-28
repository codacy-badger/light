#pragma once

#include <iostream>

#include "lexer/lexer.cpp"

using namespace std;

class ASTNode {
	public:
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

		void tabs (int tabs) {
			for (int _count = 0; _count < tabs; _count++) {
				printf("    ");
			}
		}

	private:
		static int _count;
};
