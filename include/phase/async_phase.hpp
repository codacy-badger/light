#pragma once

#include "events.hpp"
#include "async_queue.hpp"

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

struct Async_Phase {
    Async_Queue<void*> queue;
    std::thread* thread;

    std::atomic<bool> keep_working = true;
    std::atomic<bool> is_working = false;

    Async_Phase (size_t event_id) {
        Events::add_observer(event_id, &Async_Phase::handle_event, this);
        this->thread = new std::thread(&Async_Phase::async_run, this);
    }

    void handle_event (void* data) {
        this->queue.push(data);
    }

    void async_run () {
        while (this->keep_working.load()) {
            if (!this->queue.empty()) {
                this->is_working = true;
                this->handle(this->queue.pop());
                this->is_working = false;
            } else {
                this->is_working = false;
                std::this_thread::sleep_for(1ms);
            }
        }
    }

    virtual void handle (void* data) = 0;

    bool is_done () {
        return !this->is_working.load() && this->queue.empty();
    }

    void stop () {
        this->keep_working = false;
        this->thread->join();
    }
};
