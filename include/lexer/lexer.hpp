#pragma once

#include "lexer/buffer.hpp"

#include <string>
#include <deque>

enum Token_Type {
	TOKEN_UNDEFINED = 0,

	TOKEN_EOF,

	TOKEN_EXCLAMATION,
	TOKEN_AT,
	TOKEN_HASH,
	TOKEN_EQUAL,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_DIV,
	TOKEN_MUL,
	TOKEN_LET,
	TOKEN_AMP,
	TOKEN_ARROW,
	TOKEN_STRUCT,
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
	Lexer* parent = NULL;

	std::deque<Buffer*> buffer_stack;
	Buffer* buffer;

	char* nextText;
	Token_Type nextType;

	Lexer (const char* filename, Lexer* parent = NULL);
	Lexer (Buffer* buffer, Lexer* parent = NULL);

	bool parse_next ();
	bool isNextType (Token_Type type);
	void skip (unsigned int count);
	char* text ();
	const char* get_name (Token_Type type);

	void check_skip (Token_Type type);
	bool optional_skip (Token_Type type);
	void report_unexpected (Token_Type expect);
	void report_unexpected ();

	bool id ();
	bool string ();
	bool number ();
	char* copyString (std::string str);
	void handleToken (Token_Type type, const char* text);
	bool skip_ignored_and_comments ();

	Lexer* push (const char* filepath);
	Lexer* pop ();
};
