#pragma once

#include "util/logger.hpp"

#include <malloc.h>
#include <string.h>

struct Scanner {
	const char* absolute_path;
	size_t current_line;
	size_t current_col;

    const char* data;
    size_t data_length;
    size_t index = 0;

	errno_t set_input_file (const char* _absolute_path) {
		this->absolute_path = _absolute_path;
		this->current_line = 1;
		this->current_col = 1;
		this->index = 0;

        FILE* file = NULL;
        auto err = fopen_s(&file, _absolute_path, "r");
        if (err != 0) return err;

        fseek(file, 0L, SEEK_END);
        this->data_length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        this->data = (char*) calloc(this->data_length, 1);

        // @TODO @Incomplete check if we need to make the buffer bigger
        auto size = fread((void*) this->data, 1, this->data_length, file);
        while (!feof(file)) {
            size += fread((void*) (this->data + size), 1, this->data_length, file);
        }

		fclose(file);
		return 0;
	}

	const char* ref () {
		return this->data + this->index;
	}

	char next () {
        auto c = this->data[this->index++];
        this->handle_location(c);
        return c;
    }

	bool has_next () {
        return this->index < this->data_length;
    }

	char peek (size_t offset = 0) {
        return this->data[this->index + offset];
    }

    bool is_next (char c) {
    	return this->data[this->index] == c;
    }

    bool is_next (const char* expected) {
    	return this->is_next(expected, strlen(expected));
    }

    bool is_next (const char* expected, size_t length) {
    	for (size_t i = 0; i < length; i++) {
            if (this->peek(i) != expected[i])
                return false;
        }
        return true;
    }

    void skip (size_t count = 1) {
    	size_t i = 0;
    	while (this->has_next() && i < count) {
    		this->next();
    		i += 1;
    	}
    }

	char skip_and_peek (size_t count = 1) {
    	this->skip(count);
		return this->peek();
    }

    void skip_any (const char* chars) {
    	while (this->has_next()) {
    		char _c = this->peek(0);
    		if (strchr(chars, _c) == NULL) {
    			break;
    		} else this->skip();
    	}
    }

    void skip_until (const char* stopper) {
    	size_t i = 0;
    	while (this->has_next()) {
    		char _c = this->next();
    		if (_c == stopper[i]) {
    			if (++i == strlen(stopper)) return;
    		} else i = 0;
    	}
    }

    void handle_location (char character) {
    	if (character == EOF) return;

    	if (character == '\n') {
    		this->current_line += 1;
			this->current_col = 1;
    	} else this->current_col += 1;
    }
};
