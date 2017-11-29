#pragma once

#include "lexer/buffer.hpp"

Buffer::Buffer (const char* filename) {
	this->stream = fopen(filename, "r");
	this->source = filename;
}

Buffer::~Buffer () {
	fclose(this->stream);
}

bool Buffer::is_valid () {
	return this->stream != NULL;
}

char Buffer::next () {
	char output;
	if (this->pushback_buffer.empty()) {
		output = fgetc(this->stream);
	} else {
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
	return !this->pushback_buffer.empty() || !feof(this->stream);
}

char Buffer::peek (unsigned int offset) {
	if (this->pushback_buffer.size() < (offset + 1))
		this->fillPushbackBuffer(offset + 1);
	if (this->pushback_buffer.size() > offset)
		return this->pushback_buffer[offset];
	else return -1;
}

bool Buffer::isNext (char c) {
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
	printf("%s:%d,%d", this->source, this->line, this->col);
}

void Buffer::fillPushbackBuffer (unsigned int limit) {
	for (unsigned int i = this->pushback_buffer.size(); i < limit; i++) {
		char c = fgetc(this->stream);
		if (c != EOF) this->pushback_buffer.push_back(c);
	}
}

void Buffer::handleLineCol (char character) {
	if (character == EOF) return;
	if (character == '\n') {
		line += 1;
		col = 1;
	} else col += 1;
}
