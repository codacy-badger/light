#pragma once

#include "array.hpp"

#include <mutex>
#include <condition_variable>

template<typename T>
struct Async_Array {
    Array<T> wrapped;
    std::mutex mutex;

    std::condition_variable* condition;

    Async_Array (std::condition_variable* condition = NULL) {
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
        return this->wrapped.size;
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
