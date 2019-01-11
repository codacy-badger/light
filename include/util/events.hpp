#pragma once

#include "event_queue.hpp"

#include <functional>
#include <vector>
#include <map>

typedef std::map<size_t, std::vector<Event_Queue*>> ObserverMap;

struct Events {
    static ObserverMap event_observers;

    static void trigger (size_t event_id) {
        Events::trigger(event_id, NULL);
    }

    template<typename T>
    static void trigger (size_t event_id, T data) {
        void* event_data = NULL;
        memcpy(&event_data, &data, sizeof(data));

        for (auto event_queue : event_observers[event_id]) {
            event_queue->push(event_id, event_data);
        }
    }

    static void bind (size_t event_id, Event_Queue* event_queue) {
        event_observers[event_id].push_back(event_queue);
    }
};
