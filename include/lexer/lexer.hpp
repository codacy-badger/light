#pragma once

#include <deque>

#include "lexer/buffer.hpp"
#include "tokens.hpp"

struct Lexer {
	Lexer* parent = NULL;

	std::deque<Buffer*> buffer_stack;
	Buffer* buffer;

	char* nextText;
	Token_Type nextType;

	Lexer (const char* filename, Lexer* parent = NULL);
	Lexer (Buffer* buffer, Lexer* parent = NULL);

	bool parse_next ();
	bool is_next_type (Token_Type type);
	void skip (unsigned int count);
	char* text ();
	const char* get_name (Token_Type type);

	bool check_skip (Token_Type type);
	bool optional_skip (Token_Type type);
	void report_unexpected (Token_Type expect);
	void report_unexpected ();

	bool id ();
	bool string ();
	bool number ();
	void handleToken (Token_Type type, const char* text);
	bool skip_ignored_and_comments ();

	Lexer* push (const char* filepath);
	Lexer* pop ();
};
