#pragma once

#include "steps/sync_pipe.hpp"

struct Code_Source {
    const char* path = NULL;
    const char* from = NULL;

    char* absolute_path = NULL;

    const char* text = NULL;
    size_t length = 0;

    Code_Source (const char* path, const char* from = NULL) {
        this->path = path;
        this->from = from;
    }
};

struct Path_Solver : Sync_Pipe {
    const char* initial_dir;

    Path_Solver() : Sync_Pipe("Path Solver") { /* empty */ }

    void setup (Build_Settings* settings) {
        this->initial_dir = settings->initial_path;
    }

    void handle (void* in) {
        auto source = reinterpret_cast<Code_Source*>(in);
        source->absolute_path = new char[MAX_PATH_LENGTH];

        if (this->find_file(source, source->absolute_path)) {
            this->pipe_out(in);
        } else printf("[ERROR] File not found: %s\n", source->path);
    }

    bool find_file (Code_Source* source, char* buffer) {
        if (source->from) {
            os_get_absolute_path_relative_to_file(source->path, source->from, buffer);
            if (os_check_file_exists(buffer)) return true;
        }

        os_get_absolute_path_relative_to(source->path, this->initial_dir, buffer);
        if (os_check_file_exists(buffer)) return true;

        return false;
    }
};
