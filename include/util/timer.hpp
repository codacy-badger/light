#pragma once

#include "platform.hpp"

#include <stack>

typedef double timer_interval;
typedef uint64_t timer_instant;

enum Timer_Type {
    TIMER_TYPE_WALL_TIME,
    TIMER_TYPE_USER_TIME,
};

struct Timer {
    std::stack<timer_instant> stack;
    Timer_Function* timer_function;

    Timer (Timer_Type type = TIMER_TYPE_WALL_TIME) {
        switch (type) {
            case TIMER_TYPE_USER_TIME: {
                timer_function = &os_get_user_time;
                break;
            }
            case TIMER_TYPE_WALL_TIME: {
                timer_function = &os_get_wall_time;
                break;
            }
            default: {
                printf("Timer type unkown!\n");
                exit(1);
            }
        }
    }

    void start () {
        stack.push(timer_function());
    }

    timer_interval stop () {
        if (!stack.empty()) {
            auto start = stack.top();
            stack.pop();

            return os_time_stop(start, timer_function);
        } else {
            fprintf(stderr, "ERROR: Time::stop called before start");
            exit(1);
        }
    }
};
