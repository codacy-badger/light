#pragma once

#include "build_settings.hpp"
#include "utils/event_queue.hpp"
#include "steps/build_steps.hpp"

#include <thread>

struct Workspace {
    size_t guid;
    Build_Settings settings;
    Build_Steps* steps = NULL;

    Event_Queue workspace_events;
    Event_Queue* events = NULL;

    std::thread* thread = NULL;
    bool keep_going = true;

    Workspace (size_t guid) { this->guid = guid; }

    void start_building (Event_Queue* event_queue) {
        this->events = event_queue;

        this->steps = new Build_Steps();
        this->steps->setup(&this->settings);
        this->steps->set_next(NULL);

        this->thread = new std::thread(&Workspace::run_async, this);
    }

    void run_async () {
        for (auto input_file : this->settings.input_files) {
            this->steps->pipe_in((void*) input_file);
        }
        bool has_progress = true;
        while (this->keep_going && has_progress) {
            has_progress &= this->steps->pump();
        }
        this->trigger(new Compiler_Event(EVENT_COMPLETE));
    }

    void send_events_to_compiler () {
        while (!this->workspace_events.empty()) {
            this->trigger(this->workspace_events.pop());
        }
    }

    void trigger (Compiler_Event* event) {
        event->workspace = this;
        this->events->push(event);
    }

    void stop () {
        this->keep_going = false;
        this->thread->join();
        this->steps->shutdown();
    }
};
