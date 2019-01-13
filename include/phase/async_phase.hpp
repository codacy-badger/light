#pragma once

#include "phase.hpp"

#include <thread>

#define DEFAULT_SLEEP_INTERVAL 100ns
#define DEFAULT_ASYNC_PRINT_FORMAT "  + %-35s %8.6fs\n"

struct Async_Phase : Phase {
    std::thread* thread = NULL;

    bool keep_working = true;
    bool is_working = false;

    Async_Phase (const char* name, size_t main_event_id)
        : Phase(name, main_event_id, DEFAULT_ASYNC_PRINT_FORMAT) { /* empty */ }

    void start () {
        Phase::start();
        if (settings->is_multithread) {
            this->thread = new std::thread(&Async_Phase::async_run, this);
        }
    }

    void async_run () {
        while (this->keep_working) {
            if (!this->event_queue.empty()) {
                this->is_working = true;
                Phase::handle_event(this->event_queue.pop());
                this->is_working = false;
            } else std::this_thread::sleep_for(DEFAULT_SLEEP_INTERVAL);
        }
    }

    bool is_done () {
        if (settings->is_multithread) {
            return !this->is_working && this->event_queue.empty();
        } else return Phase::is_done();
    }

    void stop () {
        if (settings->is_multithread && this->keep_working) {
            this->keep_working = false;
            this->thread->join();
        }
    }
};
