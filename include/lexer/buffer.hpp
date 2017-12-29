#pragma once

#include <stdio.h>
#include <deque>

#include "report.hpp"

struct Buffer {
	FILE* stream = NULL;
	std::deque<char> pushback_buffer;

	Location location;

	Buffer (const char* filename);
	~Buffer ();

	bool is_valid ();
	char next ();
	void pushback (char c);
	bool hasNext ();
	char peek (size_t offset = 0);
	bool is_next (char c);
	bool is_next (const char* expected);
	void skip (size_t count = 1);
	void skipAny (const char* chars);
	void skipUntil (const char* stopper);
	void fillPushbackBuffer (size_t limit);
	void handleLineCol (char character);
};
