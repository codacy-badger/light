#pragma once

#include "build_context.hpp"
#include "steps/build_pipeline.hpp"

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

    Workspace (const char* name);

    void start_building ();
    Compiler_Event get_next_event ();
    void stop_building ();
    void do_full_build ();
    void stop_with_errors ();

    void run_async ();

    void add_source_file (const char* absolute_path);

    void vlog_error (const char* format, va_list args);
    void log_error (const char* format, ...);
};

Workspace* create_workspace (const char* name = DEFAULT_WORKSPACE_NAME);
