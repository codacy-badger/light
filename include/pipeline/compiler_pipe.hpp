#pragma once

#include "pipe.hpp"
#include "utils/queue.hpp"
#include "utils/async_queue.hpp"

template<typename Tin, typename Tout = Tin>
struct Compiler_Pipe : Pipe {
    Async_Queue<Tin> input_queue;

    Async_Queue<Tout>* output_queue = NULL;

    bool has_pushed_work = false;
    bool keep_working = true;

    double seconds_working = 0;

    Compiler_Pipe (const char* name) : Pipe(name) { /* empty */ }

    virtual void handle (Tin input) = 0;
    virtual void on_shutdown () { /* empty */ }

    bool pump () {
        if (!this->keep_working) return false;

        if (!this->input_queue.empty()) {
            this->has_pushed_work = false;

            auto start = os_get_time();

            auto num_items = this->input_queue.size();
            for (size_t i = 0; i < num_items; i++) {
                this->handle(this->input_queue.pop());
            }
            
            this->seconds_working += os_time_stop(start);

            return this->has_pushed_work;
        } else return false;
    }

    void push_out (Tout output) {
        if (!this->keep_working) return;

        this->has_pushed_work = true;
        if (output_queue) {
            output_queue->push(output);
        }
    }

    void push_in (Tin input) {
        if (!this->keep_working) return;

        this->input_queue.push(input);
    }

    void requeue (Tin input) {
        if (!this->keep_working) return;

        this->input_queue.push(input);
    }

    void shutdown () {
        if (!this->keep_working) return;

        this->keep_working = false;
        this->on_shutdown();
    }
};
