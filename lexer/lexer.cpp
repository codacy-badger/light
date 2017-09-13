#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <vector>

#include "../buffer/file_buffer.cpp"
#include "token.cpp"

#define LEXER_DEBUG false

#define LEXER_IGNORED " \n\t"

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||       \
    c == '$' || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))

#define LITERAL_TOKEN(literal, type) if (this->buffer->isNext(literal))	{	\
	handleToken(token, type, literal); return true; }
#define FUNCTION_TOKEN(func) if (func(token)) return true;

class Lexer {
	public:
		PushbackBuffer* buffer;

		Lexer (const char* filename) {
			this->buffer = new FileBuffer(filename);
		}

		Lexer (PushbackBuffer* buffer) {
			this->buffer = buffer;
		}

		bool hasNext () {
			return this->buffer->hasNext();
		}

		bool parse_next (Token* token) {
			token->text.clear();
			this->skip_ignored_and_comments();

		    LITERAL_TOKEN("->", Token::Type::ARROW);

			LITERAL_TOKEN("=", Token::Type::EQUAL);
			LITERAL_TOKEN(";", Token::Type::STM_END);
			LITERAL_TOKEN("+", Token::Type::ADD);
			LITERAL_TOKEN("-", Token::Type::SUB);
			LITERAL_TOKEN("*", Token::Type::MUL);
			LITERAL_TOKEN("/", Token::Type::DIV);

		    LITERAL_TOKEN(".", Token::Type::DOT);
		    LITERAL_TOKEN(",", Token::Type::COMMA);
		    LITERAL_TOKEN("(", Token::Type::PAR_OPEN);
		    LITERAL_TOKEN(")", Token::Type::PAR_CLOSE);
		    LITERAL_TOKEN("{", Token::Type::BRAC_OPEN);
		    LITERAL_TOKEN("}", Token::Type::BRAC_CLOSE);
		    LITERAL_TOKEN("[", Token::Type::SQ_BRAC_OPEN);
		    LITERAL_TOKEN("]", Token::Type::SQ_BRAC_CLOSE);
			LITERAL_TOKEN("let", Token::Type::LET);
		    LITERAL_TOKEN("type", Token::Type::TYPE);
		    LITERAL_TOKEN("function", Token::Type::FUNCTION);
		    LITERAL_TOKEN("return", Token::Type::RETURN);

			FUNCTION_TOKEN(number);
			FUNCTION_TOKEN(string);
			FUNCTION_TOKEN(id);

			return false;
		}

		bool next (Token* token) {
			if (this->tokenStack.empty()) return parse_next(token);
			else {
				Token* t = this->tokenStack[0];
				this->tokenStack.erase(this->tokenStack.begin(),
					this->tokenStack.begin() + 1);
				token->copy(t);
				delete t;
				return true;
			}
		}

		string nextText () {
			if (this->next(this->token)) {
				return this->token->text.c_str();
			} else return NULL;
		}

		bool peek (Token* token, unsigned int offset) {
			if (this->tokenStack.size() < (offset + 1))
				this->fillPushbackBuffer(offset + 1);
			token->copy(this->tokenStack[offset]);
			return token->type != Token::Type::NONE;
		}

		Token::Type peekType (unsigned int offset) {
			if (this->peek(this->token, offset)) {
				if (LEXER_DEBUG) std::cout << "peekType " << offset << " -> " <<
					Token::typeToString(this->token->type) << std::endl;
				return this->token->type;
			} else return Token::Type::NONE;
		}

		bool isNextType (Token::Type type) {
			if (LEXER_DEBUG)
				std::cout << "isNextType " << Token::typeToString(type) << " -> " <<
				Token::typeToString(this->peekType(0)) << std::endl;
			return this->peekType(0) == type;
		}

		void skip (unsigned int count) {
			for (unsigned int i = 0; i < count; i++)
				this->next(this->token);
		}

		void pushback (Token* token) {
			Token* t = new Token(token);
			this->tokenStack.push_back(t);
		}

	private:
		Token* token = new Token();
		vector<Token*> tokenStack;

		bool id (Token* token) {
			token->line = this->buffer->line;
			token->col = this->buffer->col;
			char c = this->buffer->peek(0);
		    if (ALPHA(c)) {
		        while (ALPHANUM(c)) {
			        token->text.push_back(c);
					this->buffer->skip(1);
			        c = this->buffer->peek(0);
		        }
		        token->type = Token::Type::ID;
		        return true;
		    }
			token->text.clear();
		    return false;
		}

		bool string (Token* token) {
			token->line = this->buffer->line;
			token->col = this->buffer->col;
			char c = this->buffer->peek(0);
		    if (c == '"') {
				this->buffer->skip(1);

				c = this->buffer->next();
				while (c != '"') {
					if (c == '\\')
						c = this->buffer->next();
					token->text.push_back(c);
					c = this->buffer->next();
				}

		        token->type = Token::Type::STRING;
		        return true;
		    }
			token->text.clear();
		    return false;
		}

		bool number (Token* token) {
			token->line = this->buffer->line;
			token->col = this->buffer->col;
			char c = this->buffer->peek(0);
		    if (c == '+' || c == '-') {
		        token->text.push_back(c);
				this->buffer->skip(1);
		        c = this->buffer->peek(0);
		    }
		    if (DIGIT(c) || c == '.') {
		        while (DIGIT(c)) {
					token->text.push_back(c);
					this->buffer->skip(1);
			        c = this->buffer->peek(0);
		        }
		        if (c == '.') {
		            token->text.push_back(c);
		            this->buffer->skip(1);
		            c = this->buffer->peek(0);
		            while (DIGIT(c)) {
			            token->text.push_back(c);
			            this->buffer->skip(1);
			            c = this->buffer->peek(0);
		            }
		        }
		        token->type = Token::Type::NUMBER;
		        return true;
		    }
			token->text.clear();
		    return false;
		}

		void handleToken (Token* token, Token::Type type, const char* text) {
			token->line = this->buffer->line;
			token->col = this->buffer->col;
			this->buffer->skip(strlen(text));
			token->type = type;
			token->text = "";
		}

		void skip_ignored_and_comments () {
			this->buffer->skipAny(LEXER_IGNORED);
		    if (this->buffer->peek(0) == '/') {
		        if (this->buffer->peek(1) == '/') {
					this->buffer->skipUntil("\n");
					this->buffer->skipAny(LEXER_IGNORED);
		        } else if (this->buffer->peek(1) == '*') {
					this->buffer->skipUntil("*/");
					this->buffer->skipAny(LEXER_IGNORED);
		        }
		    }
		}

		void fillPushbackBuffer (unsigned int limit) {
			for (unsigned int i = this->tokenStack.size(); i < limit; i++) {
				Token* t = new Token();
				if (!this->parse_next(t)) return;
				this->tokenStack.push_back(t);
			}
		}
};
