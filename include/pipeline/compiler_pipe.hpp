#pragma once

#include "pipe.hpp"
#include "utils/queue.hpp"
#include "utils/async_queue.hpp"

template<typename Tin, typename Tout = Tin>
struct Compiler_Pipe : Pipe {
    Async_Queue<Tin> input_queue;
    Queue<Tin> to_requeue;

    Async_Queue<Tout>* output_queue = NULL;

    bool has_pushed_work = false;
    bool keep_working = true;

    Compiler_Pipe (const char* name) : Pipe(name) { /* empty */ }

    virtual void handle (Tin input) = 0;
    virtual void on_shutdown () { /* empty */ }

    bool pump () {
        if (!this->keep_working) return false;

        if (!this->input_queue.empty()) {
            this->has_pushed_work = false;
            while (!this->input_queue.empty()) {
                this->handle(this->input_queue.pop());
            }
            while (!this->to_requeue.empty()) {
                this->push_in(this->to_requeue.pop());
            }
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

        this->to_requeue.push(input);
    }

    void shutdown () {
        this->keep_working = false;
        this->on_shutdown();
    }
};
