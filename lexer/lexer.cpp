#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <vector>

#include "../buffer/file_buffer.cpp"
#include "token.cpp"

#define LEXER_DEBUG false

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||       \
    c == '$' || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define LEXER_IGNORED " \n\t"

#define LITERAL_TOKEN(literal, type) if (this->buffer->isNext(literal))	{	\
	handleToken(type, literal); return true; }
#define FUNCTION_TOKEN(func) if (func()) return true;

class Lexer {
public:
	PushbackBuffer* buffer;

	char* nextText;
	Token::Type nextType;

	Lexer (const char* filename)
	{ this->initLexer(new FileBuffer(filename)); }

	Lexer (PushbackBuffer* buffer)
	{ this->initLexer(buffer); }

	bool parse_next () {
		while (this->skip_ignored_and_comments());

	    LITERAL_TOKEN("->", Token::Type::ARROW);

		LITERAL_TOKEN("=", Token::Type::EQUAL);
		LITERAL_TOKEN(":", Token::Type::COLON);
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
	    LITERAL_TOKEN("fn", Token::Type::FUNCTION);
	    LITERAL_TOKEN("return", Token::Type::RETURN);

		FUNCTION_TOKEN(number);
		FUNCTION_TOKEN(string);
		FUNCTION_TOKEN(id);

		return false;
	}

	bool isNextType (Token::Type type) {
		if (LEXER_DEBUG)
			std::cout << "isNextType " << Token::typeToString(type) << " -> " <<
			Token::typeToString(this->nextType) << std::endl;
		return this->nextType == type;
	}

	void skip (unsigned int count) {
		for (unsigned int i = 0; i < count; i++)
			this->parse_next();
	}

	char* text () {
		char* text = this->nextText;
		this->skip(1);
		return text;
	}

private:
	void initLexer (PushbackBuffer* buffer) {
		this->buffer = buffer;
		this->parse_next();
	}

	bool id () {
		char c = this->buffer->peek(0);
	    if (ALPHA(c)) {
			std::string _buff;
	        while (ALPHANUM(c)) {
		        _buff.push_back(c);
				this->buffer->skip(1);
		        c = this->buffer->peek(0);
	        }
	        this->nextType = Token::Type::ID;
			this->nextText = copyString(_buff);
	        return true;
	    }
	    return false;
	}

	bool string () {
		char c = this->buffer->peek(0);
	    if (c == '"') {
			this->buffer->skip(1);
	        this->nextType = Token::Type::STRING;

			std::string _buff;
			c = this->buffer->next();
			while (c != '"') {
				if (c == '\\') {
					c = this->buffer->next();
					if (c == 'n') {
						_buff.push_back('\n');
						c = this->buffer->next();
						continue;
					} else if (c == 't') {
						_buff.push_back('\t');
						c = this->buffer->next();
						continue;
					}
				}
				_buff.push_back(c);
				c = this->buffer->next();
			}
			this->nextText = copyString(_buff);
	        return true;
	    } else return false;
	}

	bool number () {
		std::string _buff;
		char c = this->buffer->peek(0);
	    if (c == '+' || c == '-') {
	        _buff.push_back(c);
			this->buffer->skip(1);
	        c = this->buffer->peek(0);
	    }
	    if (DIGIT(c) || c == '.') {
	        while (DIGIT(c)) {
				_buff.push_back(c);
				this->buffer->skip(1);
		        c = this->buffer->peek(0);
	        }
	        if (c == '.') {
	            _buff.push_back(c);
	            this->buffer->skip(1);
	            c = this->buffer->peek(0);
	            while (DIGIT(c)) {
		            _buff.push_back(c);
		            this->buffer->skip(1);
		            c = this->buffer->peek(0);
	            }
	        }
	        this->nextType = Token::Type::NUMBER;
			this->nextText = copyString(_buff);
	        return true;
	    }
	    return false;
	}

	char* copyString (std::string str) {
		char* cstr = new char[str.length() + 1];
		strcpy(cstr, str.c_str());
		return cstr;
	}

	void handleToken (Token::Type type, const char* text) {
		this->buffer->skip(strlen(text));
		this->nextText = nullptr;
		this->nextType = type;
	}

	bool skip_ignored_and_comments () {
		this->buffer->skipAny(LEXER_IGNORED);
	    if (this->buffer->peek(0) == '/') {
	        if (this->buffer->peek(1) == '/') {
				this->buffer->skipUntil("\n");
				this->buffer->skipAny(LEXER_IGNORED);
				return true;
	        } else if (this->buffer->peek(1) == '*') {
				this->buffer->skipUntil("*/");
				this->buffer->skipAny(LEXER_IGNORED);
				return true;
	        }
	    }
		return false;
	}
};
