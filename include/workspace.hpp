#pragma once

#include "build_context.hpp"
#include "pipeline/build_pipeline.hpp"

#include <thread>

#define DEFAULT_WORKSPACE_NAME "default"

struct Workspace {
    size_t guid;
    const char* name = NULL;

    Build_Context context;
    Build_Pipeline pipeline;

    std::thread* thread = NULL;
    bool keep_going = true;
    bool has_error = false;
    bool is_build_complete = false;

    Workspace (const char* name);

    void start_building ();
    Compiler_Event get_next_event ();
    void stop_building ();
    void wait_for_full_build ();
    void stop_with_errors ();

    void run_async ();

    void add_source_file (const char* absolute_path);
    void add_source_text (const char* text);
    void add_source_text (const char* text, size_t length);

    static Workspace* create_workspace (const char* name = DEFAULT_WORKSPACE_NAME);
};
