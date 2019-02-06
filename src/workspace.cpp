#include "workspace.hpp"

#include <stdarg.h>

Workspace::Workspace (const char* name) {
    static size_t next_workspace_guid = 1;
    this->guid = next_workspace_guid++;
    this->name = name;
}

void Workspace::start_building () {
    printf("Starting workspace #%zd (%s)\n", this->guid, this->name);

    this->pipeline.set_context(&this->context);
    this->pipeline.setup();
    this->pipeline.set_next(NULL);

    this->thread = new std::thread(&Workspace::run_async, this);
}

void Workspace::do_full_build () {
    this->start_building();
    while (true) {
        auto event = this->get_next_event();
        if (event.kind == EVENT_UNDEFINED) continue;

        if (event.kind == EVENT_COMPLETE) break;
        if (event.kind == EVENT_ERROR) {
            this->has_error = true;
            break;
        }
    }
    this->stop_building();
}

void Workspace::stop_building () {
    this->keep_going = false;
    this->pipeline.shutdown();
    this->thread->join();
    delete this->thread;

    printf("Workspace #%zd (%s) complete\n", this->guid, this->name);
}

void Workspace::stop_with_errors () {
    this->context.events.push(Compiler_Event(EVENT_ERROR));
}

void Workspace::run_async () {
    for (auto input_file : this->context.input_files) {
        this->add_source_file(input_file);
    }

    bool has_progress = true;
    while (this->keep_going && has_progress) {
        has_progress &= this->pipeline.pump();
    }

    this->context.events.push(Compiler_Event(EVENT_COMPLETE));
}

Compiler_Event Workspace::get_next_event () {
	if (!this->context.events.empty()) {
		return this->context.events.pop();
	} else return Compiler_Event();
}

void Workspace::add_source_file (const char* absolute_path) {
    this->pipeline.pipe_in((void*) absolute_path);
}

void Workspace::vlog_error (const char* format, va_list args) {
    printf("[ERROR] ");
    vprintf(format, args);
    printf("\n");
}

void Workspace::log_error (const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog_error(format, args);
    va_end(args);
}

Workspace* create_workspace (const char* name) {
    return new Workspace(name);
}
