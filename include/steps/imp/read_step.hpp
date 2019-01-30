#pragma once

#include "steps/sync_pipe.hpp"

#include "steps/imp/path_solver.hpp"

struct Read_Step : Sync_Pipe {

    Read_Step() : Sync_Pipe("Read Source") {}

    void handle (void* in) {
        auto source = reinterpret_cast<Code_Source*>(in);

        this->read_full_source(source);
        this->pipe_out(in);
    }

    void read_full_source (Code_Source* source) {
        FILE* file = NULL;

        this->trigger_file_open(source->absolute_path);
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
        this->trigger_file_close(source->absolute_path);
    }

    void trigger_file_open (const char* absolute_path) {
        auto file_event = new Compiler_Event_File(FILE_OPEN);
        file_event->absolute_path = absolute_path;
        //this->events->push(file_event);
    }

    void trigger_file_close (const char* absolute_path) {
        auto file_event = new Compiler_Event_File(FILE_CLOSE);
        file_event->absolute_path = absolute_path;
        //this->events->push(file_event);
    }
};
