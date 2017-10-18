#pragma once

#include <stdio.h>
#include <deque>

struct Buffer {
	FILE* stream = NULL;
	std::deque<char> pushback_buffer;

	const char* source = NULL;
	unsigned int line = 1, col = 1;

	Buffer (const char* filename);
	~Buffer ();

	bool is_valid ();
	char next ();
	void pushback (char c);
	bool hasNext ();
	char peek (unsigned int offset);
	bool isNext (char c);
	bool isNext (const char* expected);
	void skip (unsigned int count);
	void skipAny (const char* chars);
	void skipUntil (const char* stopper);
	void printLocation ();
	void fillPushbackBuffer (unsigned int limit);
	void handleLineCol (char character);
};
