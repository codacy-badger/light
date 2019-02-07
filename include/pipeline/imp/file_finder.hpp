#pragma once

#include "platform.hpp"
#include "build_context.hpp"

#define DEFAULT_SOURCE_EXTENSION ".li"

struct File_Finder {
    char base_search_path[MAX_PATH_LENGTH];

    void init (Build_Context* context) {
        memcpy(this->base_search_path, context->base_path, MAX_PATH_LENGTH);
    }

    void find_file (char* output, const char* path, bool is_module = false) {
        if (is_module) {
            // TODO: search modules path
        } else {
            // TODO: search base path
            // TODO: search relative path
            sprintf_s(output, MAX_PATH_LENGTH, "%s%s" DEFAULT_SOURCE_EXTENSION, this->base_search_path, path);
    		printf("Base path search: %s\n", output);

            os_get_absolute_path(output, output);
    		if (!os_check_file_exists(output)) {
                printf("Not found: %s\n", output);
    		} else printf("FOUND: %s\n", output);
        }
    }
};
