#pragma once

#include <functional>
#include <vector>
#include <map>

using namespace std;

typedef void (*ObserverFunction)(void*);
typedef function<void(void*)> ObserverStdFunction;
typedef map<size_t, vector<ObserverStdFunction>> ObserverMap;

struct Events {
    static ObserverMap event_observers;

    static void trigger (size_t event_id) {
        trigger(event_id, NULL);
    }

    template<typename T>
    static void trigger (size_t event_id, T data) {
        for (auto observer : event_observers[event_id]) {
            void* event_data = NULL;
            memcpy(&event_data, &data, sizeof(data));
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
        add_observer(event_id, bind(observer, instance, std::placeholders::_1));
    }

    static vector<ObserverStdFunction>* get_observers (size_t event_id) {
        return &(event_observers[event_id]);
    }

    static void remove_observers (size_t event_id) {
        event_observers[event_id].clear();
    }
};
