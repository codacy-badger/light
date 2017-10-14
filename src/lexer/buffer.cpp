#pragma once

#include "lexer/buffer.hpp"

Buffer::Buffer (istream* stream, const char* source) {
	this->stream = stream;
	this->source = source;
}

Buffer::Buffer (const char* filename) {
	this->stream = new ifstream(filename, ifstream::in);
	this->source = filename;
}

Buffer::~Buffer () {
	delete stream;
}

int Buffer::next () {
	int output = 0;
	if (this->pushback_buffer.empty()) output = this->stream->get();
	else {
		output = this->pushback_buffer.front();
		this->pushback_buffer.pop_front();
	}
	this->handleLineCol(output);
	return output;
}

void Buffer::pushback (char c) {
	this->pushback_buffer.push_back(c);
}

bool Buffer::hasNext () {
	return !this->pushback_buffer.empty() || !this->stream->eof();
}

int Buffer::peek (unsigned int offset) {
	if (this->pushback_buffer.size() < (offset + 1))
		this->fillPushbackBuffer(offset + 1);
	return this->pushback_buffer[offset];
}

bool Buffer::isNext (int c) {
	return this->peek(0) == c;
}

bool Buffer::isNext (const char* expected) {
	this->fillPushbackBuffer(strlen(expected));
	for (unsigned int i = 0; i < strlen(expected); i++) {
        if (this->peek(i) != expected[i])
            return false;
    }
    return true;
}

void Buffer::skip (unsigned int count) {
	unsigned int i = 0;
	while (this->hasNext() && i < count) {
		this->next();
		i += 1;
	}
}

void Buffer::skipAny (const char* chars) {
	while (this->hasNext()) {
		char _c = this->peek(0);
		if (strchr(chars, _c) == NULL) {
			break;
		} else this->next();
	}
}

void Buffer::skipUntil (const char* stopper) {
	unsigned int i = 0;
	while (this->hasNext()) {
		char _c = this->next();
		if (_c == stopper[i]) {
			if (++i == strlen(stopper)) return;
		} else i = 0;
	}
}

void Buffer::printLocation () {
	cout << "'" << this->source << "' @ "
		<< this->line << ", " << this->col;
}

void Buffer::fillPushbackBuffer (unsigned int limit) {
	for (unsigned int i = this->pushback_buffer.size(); i < limit; i++) {
		this->pushback_buffer.push_back(this->stream->get());
	}
}

void Buffer::handleLineCol (int character) {
	if (character == EOF) return;
	if (character == '\n') {
		line += 1;
		col = 1;
	} else col += 1;
}
