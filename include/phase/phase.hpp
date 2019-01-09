#pragma once

#include "events.hpp"
#include "compiler_settings.hpp"

#include <chrono>

typedef std::chrono::duration<double> interval;

#define print_extra_metric(name, type, value) printf("\t%-25s " type "\n", name, value)

struct Phase {
    const char* name = NULL;
    interval work_time = 0ms;

    Compiler_Settings* settings = NULL;

    Phase (const char* name, size_t event_id) {
        this->name = name;
        Events::add_observer(event_id, &Phase::handle_event, this);
    }

    virtual void handle_event (void* data) {
        auto start = std::chrono::high_resolution_clock::now();

        this->on_event(data);

        auto stop = std::chrono::high_resolution_clock::now();
        this->work_time += std::chrono::duration_cast<interval>(stop - start);
    }

    virtual void on_event (void* data) = 0;

    virtual bool is_done () { return true; }

    virtual void stop () { /* empty */ }

    void print_metrics () {
        printf("  - %-25s %8.6fs\n", this->name, this->work_time.count());
        this->print_extra_metrics();
    }

    virtual void print_extra_metrics () { /* empty */ }
};
