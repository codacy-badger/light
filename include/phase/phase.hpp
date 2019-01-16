#pragma once

#include "util/event_queue.hpp"
#include "util/events.hpp"
#include "compiler_settings.hpp"

#define print_extra_metric(name, type, value) printf("\t%-35s " type "\n", name, value)

typedef std::function<void(void*)> ObserverStdFunction;

struct Phase {
    const char* name = NULL;

    char prefix = '-';
    bool is_async = false;

    std::map<size_t, ObserverStdFunction> function_map;
    Event_Queue* event_queue;
    size_t event_from_id = 0;
    size_t event_to_id = 0;

    double work_time = 0;
    double sleep_time = 0;
    size_t sleep_count = 0;

    // async
    std::thread* thread = NULL;
    bool keep_working = true;
    bool is_working = false;

    std::condition_variable condition;
    std::mutex mutex;

    Phase (const char* name, size_t main_event_id, bool is_async = false) {
        this->event_queue = new Event_Queue(&this->condition);
        this->event_from_id = main_event_id;
        this->is_async = is_async;
        this->name = name;
    }

    void start (Compiler_Settings* settings) {
        this->bind(this->event_from_id, &Phase::handle_main_event, this);
        if (this->is_async && settings->is_multithread) {
            this->thread = new std::thread(&Phase::async_run, this);
            this->is_async = true;
            this->prefix = '+';
        } else this->is_async = false;
        this->on_start(settings);
    }

    virtual void on_start (Compiler_Settings*) { /* empty */ }

    virtual void handle_main_event (void* data) = 0;

    void async_run () {
        std::unique_lock<std::mutex> lock(mutex);
        while (this->keep_working) {
            if (!this->event_queue->empty()) {
                this->is_working = true;
                while (!this->event_queue->empty()) {
                    this->handle_event(this->event_queue->pop());
                }
                this->is_working = false;
            } else {
                auto start = os_get_time();
                condition.wait(lock);
                this->sleep_time += os_time_stop(start);
                this->sleep_count++;
            }
        }
    }

    void handle_event (Event event) {
        auto start = os_get_time();
        auto callback = this->function_map.find(event.id);
        if (callback != this->function_map.end()) {
            callback->second(event.data);
        } else {
            printf("Unexpected event: can't handle this!\n");
            exit(1);
        }
        this->work_time += os_time_stop(start);
    }

    bool is_done () {
        if (this->is_async) {
            return !this->is_working && this->event_queue->empty();
        } else {
            if (!this->event_queue->empty()) {
                while (!this->event_queue->empty()) {
                    this->handle_event(this->event_queue->pop());
                }
                return false;
            } else return true;
        }
    }

    void stop () {
        if (this->is_async && this->keep_working) {
            this->keep_working = false;
            this->condition.notify_all();
            this->thread->join();
        }
    }

    template<typename T>
    void bind (size_t event_id, void (T::*method)(void*), T* instance) {
        static_assert(std::is_base_of<Phase, T>::value, "Not a sub-type of Phase");

        this->bind(event_id, std::bind(method, instance, std::placeholders::_1));
    }

    void bind (size_t event_id, ObserverStdFunction observer_function) {
        this->function_map[event_id] = observer_function;
        Events::bind(event_id, this->event_queue);
    }

    template<typename T>
    void push (T event_data) {
        Events::trigger(this->event_to_id, event_data);
    }

    void print_metrics () {
        if (this->is_async) {
            printf("  %c %-35s %8.6fs (%8.6fs / %zd)\n", this->prefix, this->name,
                this->work_time, this->sleep_time, this->sleep_count);
        } else {
            printf("  %c %-35s %8.6fs\n", this->prefix, this->name,
                this->work_time);
        }
        this->print_extra_metrics();
    }

    virtual void print_extra_metrics () { /* empty */ }
};
