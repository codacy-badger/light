#pragma once

#include "events.hpp"
#include "async_queue.hpp"
#include "compiler_settings.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono;

typedef duration<double> interval;

#define DEFAULT_SLEEP_INTERVAL 100ns
#define print_extra_metric(name, type, value) printf("\t%-25s " type "\n", name, value)

struct Async_Phase {
    const char* name = NULL;
    Async_Queue<void*> queue;
    std::thread* thread;

    bool keep_working = true;
    bool is_working = false;
    interval work_time = 0ms;

    Compiler_Settings* settings = NULL;

    Async_Phase (const char* name, size_t event_id) {
        this->name = name;
        Events::add_observer(event_id, &Async_Phase::handle_event, this);

        this->thread = new std::thread(&Async_Phase::async_run, this);
    }

    void handle_event (void* data) {
        this->queue.push(data);
    }

    void async_run () {
        while (this->keep_working) {
            if (!this->queue.empty()) {
                this->is_working = true;

                auto start = high_resolution_clock::now();
                this->handle(this->queue.pop());
                auto stop = high_resolution_clock::now();
                this->work_time += duration_cast<interval>(stop - start);

                this->is_working = false;
            } else {
                std::this_thread::sleep_for(DEFAULT_SLEEP_INTERVAL);
            }
        }
    }

    virtual void handle (void* data) = 0;

    bool is_done () {
        return !this->is_working && this->queue.empty();
    }

    void stop () {
        if (this->keep_working) {
            this->keep_working = false;
            this->thread->join();
        } else printf("Already stopped!\n");
    }

    void print_metrics () {
        printf("  - %-25s %8.6fs\n", this->name, this->work_time.count());
        this->print_extra_metrics();
    }

    virtual void print_extra_metrics () { /* empty */ }
};
