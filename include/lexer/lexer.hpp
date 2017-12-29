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
	void skip (unsigned int count = 1);
	char* text ();
	const char* get_name (Token_Type type);

	bool check_skip (Token_Type type);
	bool optional_skip (Token_Type type);
	void report_unexpected (Token_Type expect);
	void report_unexpected ();

	bool next_is_id ();
	bool next_is_string ();
	bool next_is_number (size_t count = 0);
	void handle_token (Token_Type type, const char* text = NULL);
	bool skip_ignored_and_comments ();
};
