#pragma once

#include <chrono>
#include <stack>

typedef std::chrono::duration<double> timer_interval;
typedef std::chrono::high_resolution_clock::time_point timer_instant;

struct Timer {
    std::stack<timer_instant> stack;

    void start () {
        stack.push(std::chrono::high_resolution_clock::now());
    }

    timer_interval stop () {
        if (!stack.empty()) {
            auto start = stack.top();
            stack.pop();

            auto stop = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<timer_interval>(stop - start);
        } else {
            fprintf(stderr, "ERROR: Time::stop called before start");
            exit(1);
        }
    }
};
