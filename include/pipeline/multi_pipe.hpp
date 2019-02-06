#pragma once

#include "pipe.hpp"

struct Multi_Pipe : Pipe {
    std::vector<Pipe*> sub_pipes;

    Multi_Pipe () : Pipe(NULL) { /* empty */ }

    virtual void build_sub_pipes() = 0;

    void pipe_in(void* in) {
        if (!this->sub_pipes.empty()) {
            this->sub_pipes[0]->pipe_in(in);
        }
    }

    void pipe_out(void*) { /* empty */ }

    void handle (void*) { /* empty */ }

    void setup () {
        this->build_sub_pipes();
        for (auto pipe : this->sub_pipes) {
            pipe->set_context(this->context);
            pipe->setup();
        }
    }

    bool pump () {
        bool has_progress = false;
        for (auto pipe : this->sub_pipes) {
            has_progress |= pipe->pump();
        }
        return has_progress;
    }

    Pipe* add (Pipe* pipe) {
        this->sub_pipes.push_back(pipe);
        return pipe;
    }

    void set_next(Pipe* next) {
        size_t i = 0;
        for (; i < this->sub_pipes.size() - 1; i++) {
            auto n = this->sub_pipes[i + 1];
            this->sub_pipes[i]->set_next(n);
        }
        this->sub_pipes[i]->set_next(next);
    }

    void shutdown() {
        for (auto pipe : this->sub_pipes) {
            pipe->shutdown();
        }
    }
};
