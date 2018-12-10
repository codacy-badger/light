#pragma once

#include "lexer_buffer.hpp"

#define RING_BUFFER_SIZE 512
#define RING_BUFFER_SECTIONS 4
#define RING_BUFFER_SECTION_SIZE (RING_BUFFER_SIZE / RING_BUFFER_SECTIONS)

#define IDX(index) (index) % RING_BUFFER_SIZE

struct Ring_Buffer : Lexer_Buffer {
	char buffer[RING_BUFFER_SIZE];
	size_t remaining = 0;
	size_t index = 0;
	size_t last = 0;

	Ring_Buffer (const char* filename, Lexer_Buffer* parent = NULL)
		: Lexer_Buffer (filename, parent) {
			// If this rule doesn't get satisfied the sections of the buffer will
			// have different sizes, messing up the data
			ASSERT((RING_BUFFER_SIZE % RING_BUFFER_SECTIONS) == 0);

			this->remaining += fread(this->buffer, 1, RING_BUFFER_SIZE, this->file);
		}

	char next () {
		char output = (char) this->buffer[this->index++];
		this->index = IDX(this->index);
		this->refill_ring_buffer_if_needed();
		this->handle_location(output);
		this->remaining -= 1;
		return output;
	}

	bool has_next () {
		return this->remaining > 0;
	}

	char peek (size_t offset = 0) {
		return this->buffer[IDX(this->index + offset)];
	}

	void refill_ring_buffer_if_needed () {
		auto last_idx = IDX(this->last);
		auto current_idx = IDX(this->index);
		int64_t diff = current_idx - last_idx;
		if (abs(diff) >= RING_BUFFER_SECTION_SIZE) {
			this->remaining += fread(this->buffer + this->last, 1,
				RING_BUFFER_SECTION_SIZE, this->file);
			this->last = IDX(this->last + RING_BUFFER_SECTION_SIZE);
		}
	}
};
