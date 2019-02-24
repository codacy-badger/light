#pragma once

#include <queue>

template<typename T>
struct Queue {
    std::queue<T> wrapped;

    void push (T event) {
        this->wrapped.push(event);
    }

    T pop () {
        auto next_event = this->wrapped.front();
        this->wrapped.pop();
        return next_event;
    }

    size_t size () {
        return this->wrapped.size();
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
