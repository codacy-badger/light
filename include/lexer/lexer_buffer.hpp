#pragma once

#include "report.hpp"

struct Lexer_Buffer {
	FILE* file = NULL;
	Location location;

	Lexer_Buffer (const char* filename, Lexer_Buffer* parent = NULL) {
        auto loc = parent ? &parent->location : NULL;

        FILE* file_ptr = NULL;
    	auto err = fopen_s(&file_ptr, filename, "r");
    	if (err) {
    		char buf[256];
    		strerror_s(buf, sizeof buf, err);
    		report_error_stop(loc, "Cannot open file '%s': %s", filename, buf);
    	}

        this->location.filename = filename;
        this->file = file_ptr;
    }

	virtual char next () = 0;
	virtual bool has_next () = 0;
	virtual char peek (size_t offset = 0) = 0;

    bool is_next (char c) {
    	return this->peek(0) == c;
    }

    bool is_next (const char* expected, size_t length) {
    	for (unsigned int i = 0; i < length; i++) {
            if (this->peek(i) != expected[i])
                return false;
        }
        return true;
    }

    void skip (size_t count = 1) {
    	unsigned int i = 0;
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
    		} else this->next();
    	}
    }

    void skip_until (const char* stopper) {
    	unsigned int i = 0;
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
    		this->location.col = 1;
    	} else this->location.col += 1;
    }
};
