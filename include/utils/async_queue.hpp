#pragma once

#include "queue.hpp"

#include <mutex>
#include <condition_variable>

template<typename T>
struct Async_Queue {
    Queue<T> wrapped;
    std::mutex mutex;

    std::condition_variable* condition;

    Async_Queue (std::condition_variable* condition = NULL) {
        this->condition = condition;
    }

    void push (T event) {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->wrapped.push(event);
        if (this->condition) {
            this->condition->notify_one();
        }
    }

    T pop () {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->wrapped.pop();
    }

    size_t size () {
        return this->wrapped.size();
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
