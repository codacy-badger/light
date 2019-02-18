#include "workspace.hpp"

#include "build_context.hpp"
#include "pipeline/build_pipeline.hpp"

Workspace::Workspace (const char* name) {
    this->context = new Build_Context();
    this->pipeline = new Build_Pipeline();

    static size_t next_workspace_guid = 1;
    this->guid = next_workspace_guid++;
    this->name = name;
}

void Workspace::start_building () {
    printf("Starting workspace #%zd (%s)\n\n", this->guid, this->name);

    this->context->init(this);
    this->pipeline->init(this->context);

    this->thread = new std::thread(&Workspace::run_async, this);
}

void Workspace::wait_for_full_build () {
    this->start_building();
    while (!this->is_build_complete) { /* empty */ }
    this->stop_building();
}

void Workspace::stop_building () {
    this->pipeline->shutdown();
    this->keep_going = false;
    this->thread->join();
    delete this->thread;

    this->is_build_complete = true;

    if (this->has_error) {
        printf("\nErrors found, stopping compilation...\n");
    } else {
        printf("Workspace #%zd (%s) complete\n", this->guid, this->name);
    }
}

void Workspace::stop_with_errors () {
    if (!this->keep_going) return;

    this->has_error = true;
    this->pipeline->shutdown();
    this->keep_going = false;
}

void Workspace::run_async () {
    for (auto relative_path : this->context->input_files) {
        auto absolute_path = new char[MAX_PATH_LENGTH];
        os_get_absolute_path(relative_path, absolute_path);
        this->pipeline->add_source_file(absolute_path);
    }

    bool has_progress = true;
    while (this->keep_going && has_progress) {
        has_progress &= this->pipeline->pump();
    }

    this->is_build_complete = true;
}

void Workspace::add_source_file (const char* absolute_path) {
    this->pipeline->add_source_file(absolute_path);
}

void Workspace::add_source_text (const char* text) {
    this->add_source_text(text, strlen(text));
}

void Workspace::add_source_text (const char* text, size_t length) {
    this->pipeline->add_source_text(text, length);
}

Workspace* Workspace::create_workspace (const char* name) {
    return new Workspace(name);
}
