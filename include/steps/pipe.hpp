#pragma once

#include "build_settings.hpp"

struct Pipe {
    const char* name = NULL;

    Pipe (const char* name) { this->name = name; }

    virtual void pipe_in(void*) = 0;
    virtual void pipe_out(void*) = 0;
    virtual void set_next(Pipe*) = 0;

    virtual void handle (void*) = 0;
    virtual bool pump () = 0;

    virtual void shutdown() { /* empty */ }
    virtual void setup (Build_Settings*) { /* empty */ }
};
