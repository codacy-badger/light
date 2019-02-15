#pragma once

#include "pipe.hpp"
#include "utils/queue.hpp"
#include "utils/async_queue.hpp"

#include <stdarg.h>

template<typename Tin, typename Tout = Tin>
struct Compiler_Pipe : Pipe {
    Async_Queue<Tin> input_queue;
    Queue<Tin> to_requeue;

    Async_Queue<Tout>* output_queue = NULL;

    bool has_pushed_work = false;

    Compiler_Pipe (const char* name) : Pipe(name) { /* empty */ }

    virtual void handle (Tin input) = 0;

    bool pump () {
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
        this->has_pushed_work = true;
        if (output_queue) {
            output_queue->push(output);
        }
    }

    void push_in (Tin input) {
        this->input_queue.push(input);
    }

    void requeue (Tin input) {
        this->to_requeue.push(input);
    }

    void debug (const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->debug_v(format, args);
        va_end(args);
    }

    void debug (Ast* node, const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->debug_v(format, args);
        this->print_location(&node->location);
        va_end(args);
    }

    void debug (Location* location, const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->debug_v(format, args);
        this->print_location(location);
        va_end(args);
    }

    void error (const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->error_v(format, args);
        va_end(args);
    }

    void error (Ast* node, const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->error_v(format, args);
        this->print_location(&node->location);
        va_end(args);
    }

    void error (Location* location, const char* format, ...) {
        va_list args;
        va_start(args, format);
        this->error_v(format, args);
        this->print_location(location);
        va_end(args);
    }

    void debug_v (const char* format, va_list args) {
        printf("[DEBUG] ");
        vprintf(format, args);
        printf("\n");
    }

    void error_v (const char* format, va_list args) {
        printf("[ERROR] ");
        vprintf(format, args);
        printf("\n");
        this->context->workspace->stop_with_errors();
    }

    void print_location (Location* location) {
        printf("\t@ %s, line %zd\n", location->filename, location->line);
    }
};
