#pragma once

#include <deque>

#include "lexer/ring_buffer.hpp"
#include "tokens.hpp"

struct Lexer {
	Ring_Buffer* buffer;

	char* next_text;
	Token_Type next_type;

	Lexer (FILE* file, const char* filename);

	bool parse_next ();
	bool is_next_type (Token_Type type);
	void skip (unsigned int count = 1);
	char* text ();
	const char* get_name (Token_Type type);

	bool check_skip (Token_Type type);
	bool optional_skip (Token_Type type);
	void report_unexpected (Token_Type expect = TOKEN_EOF);

	bool next_is_id ();
	bool next_is_string ();
	bool next_is_number ();
	void handle_token (Token_Type type, const char* text = NULL);
	bool skip_ignored_and_comments ();
};
