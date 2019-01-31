#pragma once

#include <malloc.h>
#include <string.h>

struct Scanner {
	const char* absolute_path;
	size_t current_line;
	size_t current_col;

    const char* data;
    size_t data_length;
    size_t index = 0;

	void set_source (Code_Source* source) {
		this->current_line = 1;
		this->current_col = 1;
		this->index = 0;

		this->data = source->text;
		this->data_length = source->length;
		this->absolute_path = source->absolute_path;
	}

	const char* current_location_pointer () {
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

	char peek () {
		return this->data[this->index];
    }

	char peek (size_t offset) {
		auto idx = this->index + offset;
		if (idx > this->data_length) {
			return EOF;
		} else return this->data[idx];
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
