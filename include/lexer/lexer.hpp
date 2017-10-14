#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <vector>

#include "lexer/buffer.hpp"

enum Token_Type {
	TOKEN_TYPE_UNDEFINED = 0,

	TOKEN_TYPE_EQUAL,
	TOKEN_TYPE_ADD,
	TOKEN_TYPE_SUB,
	TOKEN_TYPE_DIV,
	TOKEN_TYPE_MUL,
	TOKEN_TYPE_LET,
	TOKEN_TYPE_AMP,
	TOKEN_TYPE_ARROW,
	TOKEN_TYPE_TYPE,
	TOKEN_TYPE_FUNCTION,
	TOKEN_TYPE_STM_END,
	TOKEN_TYPE_RETURN,
	TOKEN_TYPE_PAR_OPEN,
	TOKEN_TYPE_PAR_CLOSE,
	TOKEN_TYPE_BRAC_OPEN,
	TOKEN_TYPE_BRAC_CLOSE,
	TOKEN_TYPE_SQ_BRAC_OPEN,
	TOKEN_TYPE_SQ_BRAC_CLOSE,
	TOKEN_TYPE_COLON,
	TOKEN_TYPE_COMMA,
	TOKEN_TYPE_DOT,
	TOKEN_TYPE_ID,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,
};

struct Lexer {
	Buffer* buffer;

	char* nextText;
	Token_Type nextType;

	Lexer (const char* filename);
	Lexer (Buffer* buffer);

	bool parse_next ();
	bool isNextType (Token_Type type);
	void skip (unsigned int count);
	char* text ();
	bool id ();
	bool string ();
	bool number ();
	char* copyString (std::string str);
	void handleToken (Token_Type type, const char* text);
	bool skip_ignored_and_comments ();
};
