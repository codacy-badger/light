#pragma once

#include "lexer_buffer.hpp"
#include "report.hpp"

#define LEXER_BUFFER_SIZE 256

#define CONSUME(check, c) while (check(c)) { this->_buffer[count++] = c;		\
	this->skip(); c = this->peek(); }											\
	ASSERT(count < LEXER_BUFFER_SIZE);

struct File_Buffer : Lexer_Buffer {
    FILE* file = NULL;
	size_t remaining = 0;
	size_t index = 0;

	File_Buffer (const char* filename) {
        this->location.filename = filename;

    	auto err = fopen_s(&this->file, filename, "r");
    	if (err) {
    		strerror_s(this->_buffer, sizeof this->_buffer, err);
    		report_error_and_stop(NULL, "Cannot open file '%s': %s", filename, this->_buffer);
    	}
    }

	~File_Buffer () {
		fclose(this->file);
	}

    bool has_next () {
        return !feof(this->file);
    }
};
