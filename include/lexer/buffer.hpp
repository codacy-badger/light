#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

using namespace std;

struct Buffer {
	istream* stream;
	deque<int> pushback_buffer;

	const char* source;
	unsigned int line = 1, col = 1;

	Buffer (istream* stream, const char* source = "<buffer>");
	Buffer (const char* filename);
	~Buffer ();

	int next ();
	void pushback (char c);
	bool hasNext ();
	int peek (unsigned int offset);
	bool isNext (int c);
	bool isNext (const char* expected);
	void skip (unsigned int count);
	void skipAny (const char* chars);
	void skipUntil (const char* stopper);
	void printLocation ();
	void fillPushbackBuffer (unsigned int limit);
	void handleLineCol (int character);
};
