#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <vector>

#include "lexer/buffer.hpp"

enum Token_Type {
	TOKEN_UNDEFINED = 0,

	TOKEN_EOF,

	TOKEN_EQUAL,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_DIV,
	TOKEN_MUL,
	TOKEN_LET,
	TOKEN_AMP,
	TOKEN_ARROW,
	TOKEN_TYPE,
	TOKEN_FUNCTION,
	TOKEN_STM_END,
	TOKEN_RETURN,
	TOKEN_IMPORT,
	TOKEN_PAR_OPEN,
	TOKEN_PAR_CLOSE,
	TOKEN_BRAC_OPEN,
	TOKEN_BRAC_CLOSE,
	TOKEN_SQ_BRAC_OPEN,
	TOKEN_SQ_BRAC_CLOSE,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_ID,
	TOKEN_NUMBER,
	TOKEN_STRING,
};

struct Lexer {
	Buffer* buffer;
	Lexer* parent = NULL;

	char* nextText;
	Token_Type nextType;

	Lexer (const char* filename);
	Lexer (Buffer* buffer);

	bool parse_next ();
	bool isNextType (Token_Type type);
	void skip (unsigned int count);
	char* text ();
	const char* get_name (Token_Type type);

	void check_skip (Token_Type type);
	void optional_skip (Token_Type type);
	void report_unexpected (Token_Type expect);
	void report_unexpected ();

	bool id ();
	bool string ();
	bool number ();
	char* copyString (std::string str);
	void handleToken (Token_Type type, const char* text);
	bool skip_ignored_and_comments ();
};
