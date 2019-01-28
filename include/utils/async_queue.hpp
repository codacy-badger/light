#pragma once

#include <queue>
#include <mutex>

template<typename T>
struct Async_Queue {
    std::queue<T> wrapped;
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
        auto next_event = this->wrapped.front();
        this->wrapped.pop();
        return next_event;
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
