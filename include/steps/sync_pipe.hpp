#pragma once

#include "pipe.hpp"

struct Sync_Pipe : Pipe {
    Pipe* next = NULL;

    Async_Queue<void*> input_queue;

    Sync_Pipe (const char* name) : Pipe(name) { /* empty */ }

    void pipe_in(void* in) {
        this->input_queue.push(in);
    }

    bool pump () {
        if (!this->input_queue.empty()) {
            while (!this->input_queue.empty()) {
                this->handle(this->input_queue.pop());
            }
            return true;
        } else return false;
    }

    void pipe_out(void* in) {
        if (this->next) this->next->pipe_in(in);
    }

    void set_next(Pipe* n) {
        this->next = n;
    }
};
