#pragma once

#include "lexer_buffer.hpp"
#include "report.hpp"

#include <stdio.h>

struct File_Buffer : Lexer_Buffer {
    FILE* file = NULL;

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

    char next () {
        auto c = (char) fgetc(this->file);
		this->handle_location(c);
        return c;
    }

    bool has_next () {
        return this->peek() != EOF;
    }

    char peek (size_t offset = 0) {
        auto start_location = ftell(this->file);
        auto signed_offset = (long) offset;

        if (signed_offset != 0) {
            fseek(this->file, signed_offset, SEEK_CUR);
        }
        auto c = (char) fgetc(this->file);
        fseek(this->file, start_location, SEEK_SET);

        return c;
    }
};
