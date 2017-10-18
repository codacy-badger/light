#pragma once

#include "lexer/lexer.hpp"

#include <assert.h>

#define LEXER_DEBUG false

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define LEXER_IGNORED " \n\t"

#define LITERAL_TOKEN(literal, type) if (this->buffer->isNext(literal))	{	\
	handleToken(type, literal); return true; }
#define FUNCTION_TOKEN(func) if (func()) return true;

Lexer::Lexer (const char* filename) {
	this->buffer = new Buffer(filename);
	this->parse_next();
}

Lexer::Lexer (Buffer* buffer) {
	this->buffer = buffer;
	this->parse_next();
}

bool Lexer::parse_next () {
	while (this->skip_ignored_and_comments());

	if (!this->buffer->hasNext()) {
		this->nextText = NULL;
		this->nextType = TOKEN_EOF;
		return false;
	}

    LITERAL_TOKEN("->", TOKEN_ARROW);

	LITERAL_TOKEN("=", TOKEN_EQUAL);
	LITERAL_TOKEN(":", TOKEN_COLON);
	LITERAL_TOKEN(";", TOKEN_STM_END);
	LITERAL_TOKEN("+", TOKEN_ADD);
	LITERAL_TOKEN("-", TOKEN_SUB);
	LITERAL_TOKEN("*", TOKEN_MUL);
	LITERAL_TOKEN("/", TOKEN_DIV);

    LITERAL_TOKEN(".", TOKEN_DOT);
    LITERAL_TOKEN(",", TOKEN_COMMA);
    LITERAL_TOKEN("(", TOKEN_PAR_OPEN);
    LITERAL_TOKEN(")", TOKEN_PAR_CLOSE);
    LITERAL_TOKEN("{", TOKEN_BRAC_OPEN);
    LITERAL_TOKEN("}", TOKEN_BRAC_CLOSE);
    LITERAL_TOKEN("[", TOKEN_SQ_BRAC_OPEN);
    LITERAL_TOKEN("]", TOKEN_SQ_BRAC_CLOSE);
	LITERAL_TOKEN("&", TOKEN_AMP);
	LITERAL_TOKEN("let", TOKEN_LET);
    LITERAL_TOKEN("type", TOKEN_TYPE);
    LITERAL_TOKEN("fn", TOKEN_FUNCTION);
    LITERAL_TOKEN("return", TOKEN_RETURN);
    LITERAL_TOKEN("import", TOKEN_IMPORT);

	FUNCTION_TOKEN(number);
	FUNCTION_TOKEN(string);
	FUNCTION_TOKEN(id);

	fprintf(stderr, "Unrecognized token!\n");
	fprintf(stderr, " -> '%d'\n", this->buffer->peek(0));
	fprintf(stderr, "\tat %s:%d,%d\n", this->buffer->source,
		this->buffer->line, this->buffer->col);
	exit(EXIT_FAILURE);
	return false;
}

bool Lexer::isNextType (Token_Type type) {
	return this->nextType == type;
}

void Lexer::skip (unsigned int count) {
	for (unsigned int i = 0; i < count; i++)
		this->parse_next();
}

char* Lexer::text () {
	char* text = this->nextText;
	this->skip(1);
	return text;
}

#define CASE_ENUM_TEXT(T, str) case T: return str;
const char* Lexer::get_name (Token_Type type) {
	switch (type) {
		CASE_ENUM_TEXT(TOKEN_UNDEFINED, "-UNDEFINED-")
		CASE_ENUM_TEXT(TOKEN_EOF, "EOF")
		CASE_ENUM_TEXT(TOKEN_EQUAL, "=")
		CASE_ENUM_TEXT(TOKEN_ADD, "+")
		CASE_ENUM_TEXT(TOKEN_SUB, "-")
		CASE_ENUM_TEXT(TOKEN_DIV, "/")
		CASE_ENUM_TEXT(TOKEN_MUL, "*")
		CASE_ENUM_TEXT(TOKEN_LET, "let")
		CASE_ENUM_TEXT(TOKEN_AMP, "&")
		CASE_ENUM_TEXT(TOKEN_ARROW, "->")
		CASE_ENUM_TEXT(TOKEN_TYPE, "type")
		CASE_ENUM_TEXT(TOKEN_FUNCTION, "fn")
		CASE_ENUM_TEXT(TOKEN_STM_END, ";")
		CASE_ENUM_TEXT(TOKEN_RETURN, "return")
		CASE_ENUM_TEXT(TOKEN_IMPORT, "import")
		CASE_ENUM_TEXT(TOKEN_PAR_OPEN, "(")
		CASE_ENUM_TEXT(TOKEN_PAR_CLOSE, ")")
		CASE_ENUM_TEXT(TOKEN_BRAC_OPEN, "{")
		CASE_ENUM_TEXT(TOKEN_BRAC_CLOSE, "}")
		CASE_ENUM_TEXT(TOKEN_SQ_BRAC_OPEN, "[")
		CASE_ENUM_TEXT(TOKEN_SQ_BRAC_CLOSE, "]")
		CASE_ENUM_TEXT(TOKEN_COLON, ":")
		CASE_ENUM_TEXT(TOKEN_COMMA, ",")
		CASE_ENUM_TEXT(TOKEN_DOT, ".")
		CASE_ENUM_TEXT(TOKEN_ID, "<id>")
		CASE_ENUM_TEXT(TOKEN_NUMBER, "<number>")
		CASE_ENUM_TEXT(TOKEN_STRING, "<string>")
	}
	return "---";
}

void Lexer::check_skip (Token_Type type) {
	if (this->nextType != type) {
		this->report_unexpected(type);
	} else this->skip(1);
}

void Lexer::optional_skip (Token_Type type) {
	if (this->nextType == type) this->skip(1);
}

void Lexer::report_unexpected (Token_Type expected) {
	fprintf(stderr, "[ERROR] Parser: Expected '%s', but got '%s'\n",
		get_name(expected), get_name(this->nextType));
	fprintf(stderr, "\tat %s:%d,%d\n", this->buffer->source,
		this->buffer->line, this->buffer->col);
	exit(EXIT_FAILURE);
}

void Lexer::report_unexpected () {
	fprintf(stderr, "[ERROR] Parser: Unexpected token '%s'\n",
		get_name(this->nextType));
	fprintf(stderr, "\tat %s:%d,%d\n", this->buffer->source,
		this->buffer->line, this->buffer->col);
	exit(EXIT_FAILURE);
}

bool Lexer::id () {
	char c = this->buffer->peek(0);
    if (ALPHA(c)) {
		std::string _buff;
        while (ALPHANUM(c)) {
	        _buff.push_back(c);
			this->buffer->skip(1);
	        c = this->buffer->peek(0);
        }
        this->nextType = TOKEN_ID;
		this->nextText = copyString(_buff);
        return true;
    }
    return false;
}

bool Lexer::string () {
	char c = this->buffer->peek(0);
    if (c == '"') {
		this->buffer->skip(1);
        this->nextType = TOKEN_STRING;

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

bool Lexer::number () {
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
        this->nextType = TOKEN_NUMBER;
		this->nextText = copyString(_buff);
        return true;
    }
    return false;
}

char* Lexer::copyString (std::string str) {
	char* cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	return cstr;
}

void Lexer::handleToken (Token_Type type, const char* text) {
	this->buffer->skip(strlen(text));
	this->nextText = NULL;
	this->nextType = type;
}

bool Lexer::skip_ignored_and_comments () {
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
