#pragma once

#include "steps/step.hpp"

struct Read_Step : Step<const char*> {

    Read_Step() : Step("Read Source") {}

    void run (const char* absolute_path) {
        FILE* file = NULL;

        this->trigger_file_open(absolute_path);
        auto error_code = fopen_s(&file, absolute_path, "r");
        if (error_code != 0) {
			char buffer[256];
			strerror_s(buffer, sizeof buffer, error_code);
			printf("Cannot open file '%s': %s", absolute_path, buffer);
            return;
		}

        fseek(file, 0L, SEEK_END);
        auto length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        auto source_code = (char*) calloc(length, 1);

        // @TODO @Incomplete check if we need to make the buffer bigger
        auto size = fread((void*) source_code, 1, length, file);
        while (!feof(file)) {
            size += fread((void*) (source_code + size), 1, length, file);
        }

		fclose(file);
        this->trigger_file_close(absolute_path);

        this->push_out(source_code);
    }

    void trigger_file_open (const char* absolute_path) {
        auto file_event = new Compiler_Event_File(FILE_OPEN);
        file_event->absolute_path = absolute_path;
        this->events->push(file_event);
    }

    void trigger_file_close (const char* absolute_path) {
        auto file_event = new Compiler_Event_File(FILE_CLOSE);
        file_event->absolute_path = absolute_path;
        this->events->push(file_event);
    }
};
