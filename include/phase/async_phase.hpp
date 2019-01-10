#pragma once

#include "phase.hpp"

#include "util/thread_safe_queue.hpp"

#include <thread>

#define DEFAULT_SLEEP_INTERVAL 100ns

struct Async_Phase : Phase {
    Thread_Safe_Queue<void*> queue;
    std::thread* thread;

    bool keep_working = true;
    bool is_working = false;

    Async_Phase (const char* name) : Phase(name) { /* empty */ }

    void start () {
        Phase::start();
        this->thread = new std::thread(&Async_Phase::async_run, this);
    }

    void handle_event (void* data) {
        this->queue.push(data);
    }

    void async_run () {
        while (this->keep_working) {
            if (!this->queue.empty()) {
                this->is_working = true;
                Phase::handle_event(this->queue.pop());
                this->is_working = false;
            } else std::this_thread::sleep_for(DEFAULT_SLEEP_INTERVAL);
        }
    }

    bool is_done () {
        return !this->is_working && this->queue.empty();
    }

    void stop () {
        if (this->keep_working) {
            this->keep_working = false;
            this->thread->join();
        }
    }

    const char* get_phase_name_format () {
        return "  - [*] %-31s %8.6fs\n";
    }
};
