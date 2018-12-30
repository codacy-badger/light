#pragma once

#include "report.hpp"

#include <malloc.h>
#include <string.h>

struct Scanner {
	Location location;

    const char* source;
    size_t source_length;
    size_t index = 0;

    Scanner (const char* absolute_path) {
        this->location.filename = absolute_path;
        this->location.line = 1;

        FILE* file = NULL;
        auto err = fopen_s(&file, absolute_path, "r");
        if (err) {
            char buffer[256];
            strerror_s(buffer, sizeof buffer, err);
            report_error_and_stop(NULL, "Cannot open file '%s': %s", absolute_path, buffer);
        }

        fseek(file, 0L, SEEK_END);
        this->source_length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        this->source = (char*) calloc(this->source_length, 1);

        // @TODO @Incomplete check if we need to make the buffer bigger
        auto size = fread((void*) this->source, 1, this->source_length, file);
        while (!feof(file)) {
            size += fread((void*) (this->source + size), 1, this->source_length, file);
        }
    }

	char next () {
        auto c = this->source[this->index++];
        this->handle_location(c);
        return c;
    }

	bool has_next () {
        return this->index < this->source_length;
    }

	char peek (size_t offset = 0) {
        return this->source[this->index + offset];
    }

    bool is_next (char c) {
    	return this->peek() == c;
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
    		this->location.line += 1;
    	}
    }
};
