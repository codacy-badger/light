#pragma once

#include "pipe.hpp"
#include "utils/async_queue.hpp"

#include <mutex>
#include <thread>
#include <condition_variable>

struct Async_Pipe : Pipe {
    Pipe* next = NULL;

    std::condition_variable condition;
    std::mutex mutex;

    Async_Queue<void*> input_queue;
    std::thread thread;
    bool keep_working = true;
    bool is_working = false;

    Async_Pipe (const char* name) :
            Pipe(name),
            input_queue(&this->condition),
            thread(&Async_Pipe::async_handle, this)
    { /* empty */ }

    void pipe_in(void* in) {
        this->input_queue.push(in);
    }

    void async_handle () {
        std::unique_lock<std::mutex> lock(mutex);
        while (this->keep_working) {
            if (!this->input_queue.empty()) {
                this->is_working = true;
                while (!this->input_queue.empty()) {
                    this->handle(this->input_queue.pop());
                }
                this->is_working = false;
            } else condition.wait(lock);
        }
    }

    bool pump () {
        return this->is_working || !this->input_queue.empty();
    }

    void shutdown() {
        this->keep_working = false;
        this->condition.notify_one();
        this->thread.join();
    }

    void pipe_out(void* in) {
        if (this->next) this->next->pipe_in(in);
    }

    void set_next(Pipe* n) {
        this->next = n;
    }
};
