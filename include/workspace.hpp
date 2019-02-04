#pragma once

#include "code_source.hpp"
#include "build_context.hpp"
#include "steps/build_steps.hpp"

#include <thread>

struct Workspace {
    size_t guid;
    const char* name = NULL;

    Build_Context context;
    Build_Steps steps;

    std::thread* thread = NULL;
    bool keep_going = true;

    Workspace (const char* name) {
        static size_t next_workspace_guid = 1;
        this->guid = next_workspace_guid++;
        this->name = name;
    }

    void start_building () {
        printf("Starting workspace #%zd (%s)\n", this->guid, this->name);

        this->steps.set_context(&this->context);
        this->steps.setup();
        this->steps.set_next(NULL);

        this->thread = new std::thread(&Workspace::run_async, this);
    }

    void run_async () {
        for (auto input_file : this->context.input_files) {
            this->add_source_file(input_file);
        }

        bool has_progress = true;
        while (this->keep_going && has_progress) {
            has_progress &= this->steps.pump();
        }

        this->context.events.push(Compiler_Event(EVENT_COMPLETE));
    }

    Compiler_Event get_next_event () {
		if (!this->context.events.empty()) {
			return this->context.events.pop();
		} else return Compiler_Event();
	}

    void wait_for_end () {
		while (true) {
	        auto event = this->get_next_event();
            if (event.kind == EVENT_UNDEFINED) continue;

            if (event.kind == EVENT_COMPLETE) break;
	    }
	}

    void add_source_file (const char* path) {
        auto source = new Code_Source(path);
        this->steps.pipe_in((void*) source);
    }

    void add_source_text (const char* text) {
        auto source = new Code_Source(text, strlen(text));
        this->steps.pipe_in((void*) source);
    }

    void add_source_text (const char* text, size_t length) {
        auto source = new Code_Source(text, length);
        this->steps.pipe_in((void*) source);
    }

    void stop_building () {
        this->keep_going = false;
        this->steps.shutdown();
        this->thread->join();
        delete this->thread;

        printf("Workspace #%zd (%s) complete\n", this->guid, this->name);
    }

    static Workspace* create (const char* name) {
        return new Workspace(name);
    }
};
