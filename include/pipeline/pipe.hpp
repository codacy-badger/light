#pragma once

#include "build_context.hpp"

struct Pipe {
    const char* name = NULL;
    Build_Context* context = NULL;

    Pipe (const char* name) { this->name = name; }

    virtual void init () { /* empty */ }
    virtual bool pump () = 0;
    virtual void shutdown () { /* empty */ }
};
