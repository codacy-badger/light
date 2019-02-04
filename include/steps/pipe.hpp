#pragma once

#include "build_context.hpp"
#include "workspace.hpp"

struct Pipe {
    const char* name = NULL;
    Build_Context* context = NULL;

    Pipe (const char* name) { this->name = name; }

    virtual void shutdown() { /* empty */ }
    virtual void setup () { /* empty */ }

    virtual void pipe_in(void*) = 0;
    virtual void pipe_out(void*) = 0;

    virtual void handle (void*) = 0;
    virtual bool pump () = 0;

    virtual void set_next(Pipe*) = 0;
    virtual void set_context(Build_Context* c) { this->context = c; }
};
