#pragma once

#include "phase.hpp"

#include "util/async_queue.hpp"

#include <thread>

#define DEFAULT_SLEEP_INTERVAL 100ns

struct Async_Phase : Phase {
    Async_Queue<void*> queue;
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

                auto start = std::chrono::high_resolution_clock::now();
                this->on_event(this->queue.pop());
                auto stop = std::chrono::high_resolution_clock::now();
                this->work_time += std::chrono::duration_cast<interval>(stop - start);

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
