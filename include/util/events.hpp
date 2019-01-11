#pragma once

#include "event_queue.hpp"

#include <vector>
#include <map>

#define EVENT_DATA_ERROR "Event data must be register size at most (size_t)"

typedef std::map<size_t, std::vector<Event_Queue*>> ObserverMap;

struct Events {
    static ObserverMap event_observers;

    template<typename T>
    static void trigger (size_t event_id, T data) {
        static_assert(sizeof(T) <= sizeof(void*), EVENT_DATA_ERROR);

        void* event_data = NULL;
        memcpy(&event_data, &data, sizeof(T));

        for (auto event_queue : event_observers[event_id]) {
            event_queue->push(event_id, event_data);
        }
    }

    static void trigger (size_t event_id) {
        Events::trigger(event_id, NULL);
    }

    static void bind (size_t event_id, Event_Queue* event_queue) {
        event_observers[event_id].push_back(event_queue);
    }
};
