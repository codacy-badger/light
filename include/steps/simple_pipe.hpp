#pragma once

#include "pipe.hpp"

struct Simple_Pipe : Pipe {
    Pipe* next = NULL;

    Simple_Pipe (const char* name) : Pipe(name) { /* empty */ }

    void pipe_in(void* in) {
        this->handle(in);
    }

    bool pump () { return false; }

    void pipe_out(void* in) {
        if (this->next) this->next->pipe_in(in);
    }

    void set_next(Pipe* n) {
        this->next = n;
    }
};
