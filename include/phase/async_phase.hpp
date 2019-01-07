#pragma once

#include "events.hpp"

#include <thread>
#include <queue>

struct Async_Phase {
    std::queue<void*> async_queue;
    std::thread* async_thread;

    bool keep_working = true;

    Async_Phase (size_t event_id) {
        Events::add_observer(event_id, &Async_Phase::handle_event, this);
        this->async_thread = new std::thread(&Async_Phase::async_run, this);
    }

    void handle_event (void* data) {
        this->async_queue.push(data);
    }

    void async_run () {
        while (this->keep_working) {
            if (!this->async_queue.empty()) {
                auto next_item = this->async_queue.front();
                this->async_queue.pop();

                this->handle(next_item);
            } else std::this_thread::sleep_for(1ms);
        }
        printf("Async phase nicely stopped!\n");
    }

    virtual void handle (void* data) = 0;

    void stop () {
        this->keep_working = false;
        this->async_thread->join();
    }
};
