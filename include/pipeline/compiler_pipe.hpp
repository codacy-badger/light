#pragma once

#include "pipe.hpp"
#include "utils/async_queue.hpp"

#include <stdarg.h>

template<typename Tin, typename Tout = Tin>
struct Compiler_Pipe : Pipe {
    Async_Queue<Tin> input_queue;
    Async_Queue<Tout>* output_queue = NULL;

    Compiler_Pipe (const char* name) : Pipe(name) { /* empty */ }

    virtual void handle (Tin input) = 0;

    bool pump () {
        if (!this->input_queue.empty()) {
            this->handle(this->input_queue.pop());
            return true;
        } else return false;
    }

    void push_out (Tout output) {
        if (output_queue) output_queue->push(output);
    }

    void push_in (Tin input) {
        this->input_queue.push(input);
    }

    void print_error (const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->print_error_v(format, args);
        va_end(args);
    }

    void print_error_v (const char* format, va_list args) {
        printf("[ERROR] ");
        vprintf(format, args);
        printf("\n");
        this->context->workspace->stop_with_errors();
    }
};
