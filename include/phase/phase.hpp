#pragma once

#include "events.hpp"
#include "compiler_settings.hpp"
#include "util/timer.hpp"

typedef std::chrono::duration<double> interval;

#define print_extra_metric(name, type, value) printf("\t%-25s " type "\n", name, value)

struct Phase {
    const char* name = NULL;
    timer_interval work_time = 0ms;

    Compiler_Settings* settings = NULL;
    Timer timer;

    Phase (const char* name, size_t event_id) {
        this->name = name;
        Events::add_observer(event_id, &Phase::handle_event, this);
    }

    virtual void handle_event (void* data) {
        timer.start();

        this->on_event(data);

        this->work_time += timer.stop();
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
