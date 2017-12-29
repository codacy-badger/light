#include "lexer/buffer.hpp"

#include <stdio.h>
#include <string.h>

#include "report.hpp"

size_t get_ring_index (size_t index) {
	return index % RING_BUFFER_SIZE;
}

Buffer::Buffer (FILE* file, const char* filename) {
	this->location.filename = filename;
	this->file = file;

	size_t index = 0;
	while (index < RING_BUFFER_SIZE) {
		this->ring_buffer[index++] = fgetc(this->file);
	}
}

char Buffer::next () {
	char output = (char) this->ring_buffer[this->ring_buffer_index++];
	this->ring_buffer_index = get_ring_index(this->ring_buffer_index);
	this->update_ring_buffer_if_needed();
	this->handle_location(output);
	return output;
}

bool Buffer::has_next () {
	return this->ring_buffer[this->ring_buffer_index] != -1;
}

char Buffer::peek (size_t offset) {
	auto index = get_ring_index(this->ring_buffer_index + offset);
	return (char) this->ring_buffer[index];
}

bool Buffer::is_next (char c) {
	return this->peek(0) == c;
}

bool Buffer::is_next (const char* expected) {
	auto length = strlen(expected);
	for (unsigned int i = 0; i < length; i++) {
        if (this->peek(i) != expected[i])
            return false;
    }
    return true;
}

void Buffer::skip (size_t count) {
	unsigned int i = 0;
	while (this->has_next() && i < count) {
		this->next();
		i += 1;
	}
}

void Buffer::skipAny (const char* chars) {
	while (this->has_next()) {
		char _c = this->peek(0);
		if (strchr(chars, _c) == NULL) {
			break;
		} else this->next();
	}
}

void Buffer::skipUntil (const char* stopper) {
	unsigned int i = 0;
	while (this->has_next()) {
		char _c = this->next();
		if (_c == stopper[i]) {
			if (++i == strlen(stopper)) return;
		} else i = 0;
	}
}

void Buffer::update_ring_buffer_if_needed () {
	auto diff = this->ring_buffer_index - this->ring_buffer_last;
	if (diff < 0) diff = -diff;
	if (diff > (RING_BUFFER_SIZE / RING_BUFFER_SECTIONS)) {
		while (this->ring_buffer_last != this->ring_buffer_index) {
			this->ring_buffer[this->ring_buffer_last++] = fgetc(this->file);
			this->ring_buffer_last = get_ring_index(this->ring_buffer_last);
		}
	}
}

void Buffer::handle_location (char character) {
	if (character == EOF) return;
	if (character == '\n') {
		this->location.line += 1;
		this->location.col = 1;
	} else this->location.col += 1;
}
