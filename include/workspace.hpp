#pragma once

#include "compiler_events.hpp"

#include <thread>

#define DEFAULT_WORKSPACE_NAME "default"

struct Build_Context;
struct Build_Pipeline;

struct Workspace {
    size_t guid = 0;
    const char* name = NULL;

    Build_Context* context = NULL;
    Build_Pipeline* pipeline = NULL;

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
