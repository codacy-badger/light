#pragma once

#include "util/events.hpp"
#include "compiler_settings.hpp"
#include "util/timer.hpp"

using namespace std::chrono_literals;

typedef std::chrono::duration<double> interval;

#define print_extra_metric(name, type, value) printf("\t%-35s " type "\n", name, value)

struct Phase {
    const char* name = NULL;
    timer_interval work_time = 0ms;

    Compiler_Settings* settings = NULL;
    size_t event_from_id = 0;
    size_t event_to_id = 0;

    Timer timer;

    Phase (const char* name) { this->name = name; }

    virtual void start () {
        Events::add_observer(this->event_from_id, &Phase::handle_event, this);
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
        printf(this->get_phase_name_format(), this->name, this->work_time.count());
        this->print_extra_metrics();
    }

    virtual const char* get_phase_name_format () {
        return "  - %-35s %8.6fs\n";
    }

    virtual void print_extra_metrics () { /* empty */ }
};
