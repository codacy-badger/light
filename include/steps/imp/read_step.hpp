#pragma once

#include "steps/simple_pipe.hpp"

#include "code_source.hpp"
#include "compiler_events.hpp"

struct Read_Step : Simple_Pipe {

    Read_Step() : Simple_Pipe("Read Source") {}

    void handle (void* in) {
        auto source = reinterpret_cast<Code_Source*>(in);

        this->read_full_source(source);
        this->pipe_out(in);
    }

    void read_full_source (Code_Source* source) {
        FILE* file = NULL;

        auto error_code = fopen_s(&file, source->absolute_path, "r");
        if (error_code != 0) {
			char buffer[256];
			strerror_s(buffer, sizeof buffer, error_code);
			printf("Cannot open file '%s': %s", source->absolute_path, buffer);
		}

        fseek(file, 0L, SEEK_END);
        auto length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        source->text = (char*) calloc(length, 1);

        // @TODO @Incomplete check if we need to make the buffer bigger
        source->length = fread((void*) source->text, 1, length, file);
        while (!feof(file)) {
            source->length += fread((void*) (source->text + source->length), 1, length, file);
        }

		fclose(file);
    }
};
