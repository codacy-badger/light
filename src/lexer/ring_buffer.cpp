#include "lexer/ring_buffer.hpp"

#include <stdio.h>
#include <string.h>

#include "report.hpp"

size_t get_ring_index (size_t index) {
	return index % RING_BUFFER_SIZE;
}

Ring_Buffer::Ring_Buffer (FILE* file, const char* filename) {
	this->location.filename = filename;
	this->file = file;

	size_t index = 0;
	while (index < RING_BUFFER_SIZE) {
		this->ring_buffer[index++] = fgetc(this->file);
	}
}

char Ring_Buffer::next () {
	char output = (char) this->ring_buffer[this->ring_buffer_index++];
	this->ring_buffer_index = get_ring_index(this->ring_buffer_index);
	this->update_ring_buffer_if_needed();
	this->handle_location(output);
	return output;
}

bool Ring_Buffer::has_next () {
	return this->ring_buffer[this->ring_buffer_index] != -1;
}

char Ring_Buffer::peek (size_t offset) {
	auto index = get_ring_index(this->ring_buffer_index + offset);
	return (char) this->ring_buffer[index];
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

void Ring_Buffer::update_ring_buffer_if_needed () {
	auto diff = this->ring_buffer_index - this->ring_buffer_last;
	if (diff < 0) diff = -diff;
	if (diff > (RING_BUFFER_SIZE / RING_BUFFER_SECTIONS)) {
		while (this->ring_buffer_last != this->ring_buffer_index) {
			this->ring_buffer[this->ring_buffer_last++] = fgetc(this->file);
			this->ring_buffer_last = get_ring_index(this->ring_buffer_last);
		}
	}
}

void Ring_Buffer::handle_location (char character) {
	if (character == EOF) return;
	if (character == '\n') {
		this->location.line += 1;
		this->location.col = 1;
	} else this->location.col += 1;
}
