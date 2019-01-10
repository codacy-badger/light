#pragma once

#include "util/thread_safe_queue.hpp"
#include "util/events.hpp"
#include "util/timer.hpp"
#include "compiler_settings.hpp"

using namespace std::chrono_literals;

#define DEFAULT_PRINT_FORMAT "  - %-35s %8.6fs\n"
#define print_extra_metric(name, type, value) printf("\t%-35s " type "\n", name, value)

struct Phase {
    const char* name = NULL;
    const char* print_format = NULL;
    Thread_Safe_Queue<void*> queue;

    Compiler_Settings* settings = NULL;
    size_t event_from_id = 0;
    size_t event_to_id = 0;

    Timer timer;
    timer_interval work_time = 0ms;

    Phase (const char* name, const char* print_format = DEFAULT_PRINT_FORMAT) {
        this->print_format = print_format;
        this->name = name;
    }

    virtual void start () {
        Events::add_observer(this->event_from_id, &Phase::push_event, this);
    }

    // @TODO this method could be removed by referencing the
    // Queue push method directly
    void push_event (void* data) {
        this->queue.push(data);
    }

    void handle_event (void* data) {
        timer.start();
        this->on_event(data);
        this->work_time += timer.stop();
    }

    virtual void on_event (void* data) = 0;

    virtual bool is_done () {
        if (!this->queue.empty()) {
            while (!this->queue.empty()) {
                this->handle_event(this->queue.pop());
            }
            return false;
        } else return true;
    }

    virtual void stop () { /* empty */ }

    void print_metrics () {
        printf(this->print_format, this->name, this->work_time.count());
        this->print_extra_metrics();
    }

    virtual void print_extra_metrics () { /* empty */ }
};
