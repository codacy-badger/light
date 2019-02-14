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

    bool empty () {
        return this->wrapped.empty();
    }
};
