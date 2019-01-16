#pragma once

#include "event.hpp"

#include <queue>
#include <mutex>

struct Event_Queue {
    std::queue<Event> wrapped;
    std::mutex mutex;

    std::condition_variable* condition;

    Event_Queue (std::condition_variable* condition = NULL) {
        this->condition = condition;
    }

    void push (size_t event_id, void* data = NULL) {
        this->push(Event(event_id, data));
    }

    void push (Event event) {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->wrapped.push(event);
        if (this->condition) {
            this->condition->notify_one();
        }
    }

    Event pop () {
        std::lock_guard<std::mutex> lock(this->mutex);
        auto next_event = this->wrapped.front();
        this->wrapped.pop();
        return next_event;
    }

    bool empty () {
        return this->wrapped.empty();
    }
};
