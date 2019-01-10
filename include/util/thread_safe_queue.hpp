#pragma once

#include <queue>
#include <mutex>

template<typename T>
struct Thread_Safe_Queue {
    std::queue<T> wrapped;
    std::mutex mutex;

    void push (T item) {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->wrapped.push(item);
    }

    T pop () {
        std::lock_guard<std::mutex> lock(this->mutex);
        auto next_item = this->wrapped.front();
        this->wrapped.pop();
        return next_item;
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
