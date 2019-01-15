#pragma once

#include "platform.hpp"

#include <stack>

struct Timer {
    std::stack<uint64_t> stack;

    void start () {
        stack.push(os_get_time());
    }

    double stop () {
        if (!stack.empty()) {
            auto start = stack.top();
            stack.pop();

            return os_time_stop(start);
        } else {
            fprintf(stderr, "ERROR: Timer::stop called before Timer::start");
            exit(1);
        }
    }
};
