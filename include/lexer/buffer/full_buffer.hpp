#pragma once

#include <assert.h>

#include "lexer_buffer.hpp"

void replace_special_characters (char* buffer, size_t length) {
	for (size_t i = 0; i < length; i++) {
		if (buffer[i] == '\\') {
			switch (buffer[i + 1]) {
				case 'n': buffer[i] = '\n'; 			break;
				case 't': buffer[i] = '\t'; 			break;
				default:  buffer[i] = buffer[i + 1];	break;
			}

			memcpy(&buffer[i + 1], &buffer[i + 2], length);
			i += 1;
		}
	}
}

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

	~Full_Buffer () {
		free(this->buffer);
	}

	char* get_next_id () {
		char c = this->buffer[this->index];
	    if (ALPHA(c)) {
			size_t length = 1;

			c = this->buffer[this->index + length];
			while (ALPHANUM(c)) {
				length += 1;
				c = this->buffer[this->index + length];
			}
			assert(length < LEXER_BUFFER_SIZE);

			auto str = (char*) malloc(length + 1);
			memcpy(str, &this->buffer[this->index], length);
			str[length] = 0;

			this->skip(length);
			return str;
	    }
	    return NULL;
	}

	char* get_next_string () {
		char c = this->buffer[this->index];
	    if (c == '"') {
			size_t length = 1;

			c = this->buffer[this->index + length];
			while (c != '"') {
				if (c == '\\') {
					length += 2;
				} else length += 1;
				c = this->buffer[this->index + length];
			}
			assert(length < LEXER_BUFFER_SIZE);

			auto str = (char*) malloc(length);
			memcpy(str, &this->buffer[this->index + 1], length - 1);
			str[length - 1] = 0;

			replace_special_characters(str, length);
			this->skip(length + 1);
			return str;
	    } else return NULL;
	}

	char* get_next_number () {
		size_t length = 0;
		char c = this->buffer[this->index];
		if (c == '0' && this->buffer[this->index + 1] == 'x') {
			length += 2;

			c = this->buffer[this->index + length];
			while (ALPHANUM(c)) {
				length += 1;
				c = this->buffer[this->index + length];
			}
		} else if (c == '0' && this->buffer[this->index + 1] == 'b') {
			length += 2;

			c = this->buffer[this->index + length];
			while (c == '0' || c == '1') {
				length += 1;
				c = this->buffer[this->index + length];
			}
		} else {
			if (c == '+' || c == '-') {
				length += 1;
				c = this->buffer[this->index + length];
			}

			while (DIGIT(c)) {
				length += 1;
				c = this->buffer[this->index + length];
			}

			if (c == '.') {
				length += 1;
				c = this->buffer[this->index + length];
				while (DIGIT(c)) {
					length += 1;
					c = this->buffer[this->index + length];
				}
			}
		}

		if (length > 0) {
			auto str = (char*) malloc(length + 1);
			memcpy(str, &this->buffer[this->index], length);
			str[length] = 0;

			this->skip(length);
			return str;
		} else return NULL;
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
