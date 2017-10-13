#pragma once

#include <fstream>
#include <string>
#include <deque>

using namespace std;

struct Buffer {
	istream* stream;
	deque<int> pushback_buffer;

	const char* source;
	unsigned int line = 1, col = 1;

	Buffer (istream* stream, const char* source = "<buffer>") {
		this->stream = stream;
		this->source = source;
	}

	Buffer (const char* filename) {
		this->stream = new ifstream(filename, ifstream::in);
		this->source = filename;
	}

	~Buffer () {
		delete stream;
	}

	int next () {
		int output = 0;
		if (this->pushback_buffer.empty()) output = this->stream->get();
		else {
			output = this->pushback_buffer.front();
			this->pushback_buffer.pop_front();
		}
		this->handleLineCol(output);
		return output;
	}

	void pushback (char c) {
		this->pushback_buffer.push_back(c);
	}

	bool hasNext () {
		return !this->pushback_buffer.empty() || !this->stream->eof();
	}

	int peek (unsigned int offset) {
		if (this->pushback_buffer.size() < (offset + 1))
			this->fillPushbackBuffer(offset + 1);
		return this->pushback_buffer[offset];
	}

	bool isNext (int c) {
		return this->peek(0) == c;
	}

	bool isNext (const char* expected) {
		this->fillPushbackBuffer(strlen(expected));
		for (unsigned int i = 0; i < strlen(expected); i++) {
	        if (this->peek(i) != expected[i])
	            return false;
	    }
	    return true;
	}

	void skip (unsigned int count) {
		unsigned int i = 0;
		while (this->hasNext() && i < count) {
			this->next();
			i += 1;
		}
	}

	void skipAny (const char* chars) {
		while (this->hasNext()) {
			char _c = this->peek(0);
			if (strchr(chars, _c) == NULL) {
				break;
			} else this->next();
		}
	}

	void skipUntil (const char* stopper) {
		unsigned int i = 0;
		while (this->hasNext()) {
			char _c = this->next();
			if (_c == stopper[i]) {
				if (++i == strlen(stopper)) return;
			} else i = 0;
		}
	}

	void printLocation () {
		cout << "'" << this->source << "' @ "
			<< this->line << ", " << this->col;
	}

	void fillPushbackBuffer (unsigned int limit) {
		for (unsigned int i = this->pushback_buffer.size(); i < limit; i++) {
			this->pushback_buffer.push_back(this->stream->get());
		}
	}

	void handleLineCol (int character) {
		if (character == EOF) return;
		if (character == '\n') {
			line += 1;
			col = 1;
		} else col += 1;
	}
};
