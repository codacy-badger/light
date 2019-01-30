#pragma once

#include "build_settings.hpp"
#include "utils/event_queue.hpp"
#include "steps/build_steps.hpp"

#include "steps/imp/path_solver.hpp"

#include <thread>

struct Workspace {
    size_t guid;
    const char* name = NULL;

    Build_Settings settings;
    Event_Queue workspace_events;
    Build_Steps* steps = NULL;

    std::thread* thread = NULL;
    bool keep_going = true;

    Workspace (const char* name) {
        static size_t next_workspace_guid = 1;
        this->guid = next_workspace_guid++;
        this->name = name;
    }

    void start_building () {
        printf("Starting workspace #%zd (%s)\n", this->guid, this->name);

        this->steps = new Build_Steps();
        this->steps->setup(&this->settings);
        this->steps->set_next(NULL);

        this->thread = new std::thread(&Workspace::run_async, this);
    }

    void run_async () {
        for (auto input_file : this->settings.input_files) {
            this->steps->pipe_in((void*) new Code_Source(input_file));
        }
        bool has_progress = true;
        while (this->keep_going && has_progress) {
            has_progress &= this->steps->pump();
        }
        this->trigger(new Compiler_Event(EVENT_COMPLETE));
    }

    Compiler_Event* get_next_event () {
		if (!this->workspace_events.empty()) {
			return this->workspace_events.pop();
		} else return NULL;
	}

    void wait_for_end () {
		while (true) {
	        auto event = this->get_next_event();
			if (!event) continue;

			assert(event->workspace != NULL);

            switch (event->kind) {
                case EVENT_COMPLETE: return;
                case EVENT_FILE: {
                    auto file_event = static_cast<Compiler_Event_File*>(event);
    				switch (file_event->file_kind) {
    					case FILE_OPEN:  { printf(" >> OPEN  "); break; }
    					case FILE_CLOSE: { printf(" << CLOSE "); break; }
    				}
    				printf(file_event->absolute_path);
    				printf("\n");
                }
            }
	    }
	}

    void send_events_to_compiler () {
        while (!this->workspace_events.empty()) {
            this->trigger(this->workspace_events.pop());
        }
    }

    void trigger (Compiler_Event* event) {
        event->workspace = this;
        this->workspace_events.push(event);
    }

    void stop_building () {
        this->keep_going = false;
        this->steps->shutdown();
        this->thread->join();

        printf("Workspace #%zd (%s) complete\n", this->guid, this->name);
    }

    static Workspace* create (const char* name) {
        return new Workspace(name);
    }
};
