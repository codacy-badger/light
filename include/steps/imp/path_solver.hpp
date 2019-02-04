#pragma once

#include "steps/simple_pipe.hpp"
#include "code_source.hpp"

struct Path_Solver : Simple_Pipe {
    const char* initial_dir;

    Path_Solver() : Simple_Pipe("Path Solver") { /* empty */ }

    void setup () {
        this->initial_dir = this->context->initial_path;
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
            printf("\t - %s\n", buffer);
        }

        os_get_absolute_path_relative_to(source->path, this->initial_dir, buffer);
        if (os_check_file_exists(buffer)) return true;
        printf("\t - %s\n", buffer);

        return false;
    }
};
