#pragma once

#include "build_settings.hpp"
#include "utils/event_queue.hpp"
#include "steps/build_steps.hpp"

#include <thread>

struct Workspace {
    Build_Settings settings;
    const char* name = NULL;
    Build_Steps* steps = NULL;

    Event_Queue workspace_events;
    Event_Queue* events = NULL;

    std::thread* thread = NULL;
    bool keep_going = true;

    Workspace (const char* name) { this->name = name; }

    void start_building (Event_Queue* event_queue) {
        this->events = event_queue;
        
        this->steps = new Build_Steps(&this->workspace_events, &this->settings);
        this->steps->setup();

        this->thread = new std::thread(&Workspace::run_async, this);
    }

    void run_async () {
        while (this->keep_going) {
            this->steps->do_some_work();
            this->send_events_to_compiler();
        }
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
        this->steps->teardown();
    }
};
