#include "lexer/ring_buffer.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

inline
size_t get_ring_index (size_t index) {
	return index % RING_BUFFER_SIZE;
}

Ring_Buffer::Ring_Buffer (FILE* file, const char* filename) {
	// If this rule doesn't get satisfied the sections of the buffer will
	// have different sizes, messing up the data
	assert((RING_BUFFER_SIZE % RING_BUFFER_SECTIONS) == 0);

	this->location.filename = filename;
	this->location.line = 1;
	this->location.col = 1;
	this->file = file;

	this->remaining += fread(this->buffer, 1, RING_BUFFER_SIZE, this->file);
}

char Ring_Buffer::next () {
	char output = (char) this->buffer[this->index++];
	this->index = get_ring_index(this->index);
	this->refill_ring_buffer_if_needed();
	this->handle_location(output);
	this->remaining -= 1;
	return output;
}

bool Ring_Buffer::has_next () {
	return this->remaining > 0;
}

char Ring_Buffer::peek (size_t offset) {
	auto tmp = get_ring_index(this->index + offset);
	return (char) this->buffer[tmp];
}

bool Ring_Buffer::is_next (char c) {
	return this->peek(0) == c;
}

bool Ring_Buffer::is_next (const char* expected) {
	auto length = strlen(expected);
	for (unsigned int i = 0; i < length; i++) {
        if (this->peek(i) != expected[i])
            return false;
    }
    return true;
}

void Ring_Buffer::skip (size_t count) {
	unsigned int i = 0;
	while (this->has_next() && i < count) {
		this->next();
		i += 1;
	}
}

void Ring_Buffer::skip_any (const char* chars) {
	while (this->has_next()) {
		char _c = this->peek(0);
		if (strchr(chars, _c) == NULL) {
			break;
		} else this->next();
	}
}

void Ring_Buffer::skip_until (const char* stopper) {
	unsigned int i = 0;
	while (this->has_next()) {
		char _c = this->next();
		if (_c == stopper[i]) {
			if (++i == strlen(stopper)) return;
		} else i = 0;
	}
}

void Ring_Buffer::refill_ring_buffer_if_needed () {
	auto last_idx = get_ring_index(this->last);
	auto current_idx = get_ring_index(this->index);
	int64_t diff = current_idx - last_idx;
	if (abs(diff) >= RING_BUFFER_SECTION_SIZE) {
		this->remaining += fread(this->buffer + this->last, 1,
			RING_BUFFER_SECTION_SIZE, this->file);
		this->last = get_ring_index(this->last + RING_BUFFER_SECTION_SIZE);
	}
}

void Ring_Buffer::handle_location (char character) {
	if (character == EOF) return;
	if (character == '\n') {
		this->location.line += 1;
		this->location.col = 1;
	} else this->location.col += 1;
}
