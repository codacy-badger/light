#pragma once

#include "utils/event_queue.hpp"
#include "utils/ast_queue.hpp"

#include <queue>

template<typename Tin = Ast_Statement*, typename Tout = Tin>
struct Step {
    const char* name = NULL;
    Event_Queue* events = NULL;

    Async_Queue<Tin> input;
    Async_Queue<Tout>* output = NULL;

    Step (const char* name) { this->name = name; }

    virtual void setup (Build_Settings*) { /* empty */ }

    virtual void run (Tin) = 0;

    bool do_some_work () {
        auto has_worked = false;

        while (!this->input.empty()) {
            this->run(this->input.pop());
            has_worked = true;
        }

        return has_worked;
    }

    void push_out (Tout result) {
        if (this->output) {
            this->output->push(result);
        }
    }

    virtual void teardown () { /* empty */ }
};
