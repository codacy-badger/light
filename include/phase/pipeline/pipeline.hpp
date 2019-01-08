#pragma once

#include "pipe.hpp"

#include <vector>

struct Pipeline {
    std::vector<Pipe*> pipes;

    Pipeline* pipe (Pipe* pipe) {
        pipe->pipe_index = this->pipes.size();
        pipe->pipeline = this;
        this->pipes.push_back(pipe);
        return this;
    }

    void process (Ast_Scope* scope, size_t start_index = 0) {
        for (auto i = start_index; i < this->pipes.size(); i++) {
            auto pipe = this->pipes[i];
            pipe->process(scope);
        }
    }

    void print_metrics (double total_time) {
        double percent;
        for (auto pipe : this->pipes) {
            if (pipe->pipe_name) {
    			percent = (pipe->total_time * 100.0) / total_time;
    			printf("  - %-25s%8.6fs (%5.2f%%)\n", pipe->pipe_name, pipe->total_time, percent);
    			pipe->print_pipe_metrics();
    		}
        }
    	printf("\n  Total Time                 %8.6fs\n", total_time);
    }
};
