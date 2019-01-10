#pragma once

#include <functional>
#include <vector>
#include <map>

typedef void (*ObserverFunction)(void*);
typedef std::function<void(void*)> ObserverStdFunction;
typedef std::map<size_t, std::vector<ObserverStdFunction>> ObserverMap;

struct Events {
    static ObserverMap event_observers;

    static void trigger (size_t event_id) {
        trigger(event_id, NULL);
    }

    template<typename T>
    static void trigger (size_t event_id, T data) {
        void* event_data = NULL;
        memcpy(&event_data, &data, sizeof(data));

        for (auto observer : event_observers[event_id]) {
            observer(event_data);
        }
    }

    static void add_observer (size_t event_id, ObserverStdFunction observer) {
        event_observers[event_id].push_back(observer);
    }

    static void add_observer (size_t event_id, ObserverFunction observer) {
        add_observer(event_id, bind(observer, std::placeholders::_1));
    }

    template<typename T>
    static void add_observer (size_t event_id, void (T::*observer)(void*), T* instance) {
        add_observer(event_id, std::bind(observer, instance, std::placeholders::_1));
    }

    static std::vector<ObserverStdFunction>* get_observers (size_t event_id) {
        return &(event_observers[event_id]);
    }

    static void remove_observers (size_t event_id) {
        event_observers[event_id].clear();
    }
};
