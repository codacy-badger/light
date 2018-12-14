#pragma once

#include "lexer_buffer.hpp"

#include <malloc.h>

struct String_Buffer : Lexer_Buffer {
	char* buffer = NULL;
	size_t index = 0;
	size_t size = 0;

	String_Buffer (const size_t _size = 0) {
        // @TODO @Incomplete check if the malloc call suceeded
		if (_size > 0) {
			this->buffer = (char*) malloc(_size);
		}
	}

	String_Buffer (char* data)
			: Lexer_Buffer () {
        this->buffer = data;
	}

	char next () {
		char output = (char) this->buffer[this->index++];
		this->handle_location(output);
		return output;
	}

	bool has_next () {
		return this->index < this->size;
	}

	char peek (size_t offset = 0) {
        auto idx = this->index + offset;
        if (idx < this->size) {
            return this->buffer[idx];
		} else return -1;
	}
};
