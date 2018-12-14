#pragma once

#include "string_buffer.hpp"

struct Full_Buffer : String_Buffer {

	Full_Buffer (const char* filename)
			: String_Buffer () {
		FILE* file = NULL;

		auto err = fopen_s(&file, filename, "r");
    	if (err) {
    		strerror_s(this->_buffer, sizeof this->_buffer, err);
    		report_error_and_stop(NULL, "Cannot open file '%s': %s", filename, this->_buffer);
    	}

        fseek(file, 0L, SEEK_END);
        auto _size = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the malloc call suceeded
        this->buffer = (char*) malloc(_size);

        this->size = fread(this->buffer, 1, _size, file);
        while (!feof(file)) {
            this->size += fread(this->buffer + this->size, 1, _size, file);
        }

		if (this->size > _size) {
			this->buffer = (char*) realloc(this->buffer, this->size);
		}
	}

	~Full_Buffer () {
		free(this->buffer);
	}
};
