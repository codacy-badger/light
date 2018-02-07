#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "lexer_buffer.hpp"

struct Full_Buffer : Lexer_Buffer {
	char* buffer = NULL;
	size_t index = 0;
	size_t size = 0;

	Full_Buffer (const char* filename, Lexer_Buffer* parent = NULL)
		: Lexer_Buffer (filename, parent) {
            fseek(this->file, 0L, SEEK_END);
            this->size = ftell(this->file);
            rewind(this->file);

            // @TODO @Incomplete check if the malloc call suceeded
            this->buffer = (char*) malloc(this->size);

            auto read_count = fread(this->buffer, 1, this->size, this->file);
            while (!feof(this->file)) {
                read_count += fread(this->buffer + read_count, 1, this->size, this->file);
            }
            this->size = read_count;

            // @Speed do we really need to do this? Think!
            this->buffer = (char*) realloc(this->buffer, this->size);
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
