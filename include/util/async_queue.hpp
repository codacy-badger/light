#pragma once

#include <queue>
#include <mutex>

template<typename T>
struct Async_Queue {
    std::queue<T> async_queue;
    std::mutex mutex;

    void push (T item) {
        this->mutex.lock();
        this->async_queue.push(item);
        this->mutex.unlock();
    }

    T pop () {
        this->mutex.lock();
        auto next_item = this->async_queue.front();
        this->async_queue.pop();
        this->mutex.unlock();
        return next_item;
    }

    bool empty () {
        this->mutex.lock();
        auto is_empty = this->async_queue.empty();
        this->mutex.unlock();
        return is_empty;
    }
};
