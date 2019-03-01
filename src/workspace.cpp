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
    this->start_time = os_get_time();

    this->context->init(this);
    this->pipeline->init(this->context);

    this->thread = new std::thread(&Workspace::run_async, this);
}

void Workspace::wait_for_full_build () {
    this->start_building();
    while (!this->is_build_finished) { /* empty */ }
    this->stop_building();
}

void Workspace::stop_building () {
    this->pipeline->shutdown();
    this->keep_going = false;
    this->thread->join();
    delete this->thread;

    this->is_build_finished = true;

    if (this->context->has_error) {
        printf("Workspace #%zd (%s) stopped due to errors\n",
            this->guid, this->name);
    } else {
        this->total_time += os_time_stop(this->start_time);
        printf("Workspace #%zd (%s) done in %8.6fs\n",
            this->guid, this->name, this->total_time);
    }
}

void Workspace::stop_with_errors () {
    if (!this->keep_going) return;

    this->context->has_error = true;
    this->pipeline->shutdown();
    this->keep_going = false;
}

void Workspace::run_async () {
    For (this->context->input_files) {
        auto absolute_path = new char[MAX_PATH_LENGTH];
        os_get_absolute_path(it, absolute_path);
        this->pipeline->add_source_file(absolute_path);
    }

    bool has_progress = true;
    while (this->keep_going && has_progress) {
        has_progress &= this->pipeline->pump();
    }

    this->is_build_finished = true;
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
