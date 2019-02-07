#pragma once

#include "pipeline/compiler_pipe.hpp"

#include "parse_step.hpp"

struct Source_File {
    const char* absolute_path;

    Source_File (const char* absolute_path) {
        this->absolute_path = absolute_path;
    }
};

struct Read_File_Step : Compiler_Pipe<Source_File, Parse_Command> {

    Read_File_Step () : Compiler_Pipe("Read File") {}

    void handle (Source_File source_file) {
        size_t length = 0;
        auto text = this->read_full_source(source_file.absolute_path, &length);
        this->push_out(Parse_Command(text, length, source_file.absolute_path));
    }

    const char* read_full_source (const char* absolute_path, size_t* length_ptr) {
        FILE* file = NULL;

        auto error_code = fopen_s(&file, absolute_path, "r");
        if (error_code != 0) {
			char buffer[256];
			strerror_s(buffer, sizeof buffer, error_code);
			printf("Cannot open file '%s': %s", absolute_path, buffer);
		}

        fseek(file, 0L, SEEK_END);
        auto length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        auto text = (char*) malloc(length);
        memset(text, 0, length);

        // @TODO @Incomplete check if we need to make the buffer bigger
        (*length_ptr) = fread((void*) text, 1, length, file);
        while (!feof(file)) {
            (*length_ptr) += fread((void*) (text + (*length_ptr)), 1, length, file);
        }

		fclose(file);
        return text;
    }
};
