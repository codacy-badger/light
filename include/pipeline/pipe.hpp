#pragma once

#include "build_context.hpp"

struct Pipe {
    const char* name;

    Pipe (const char* name) { this->name = name; }

    virtual void init (Build_Context*) { /* empty */ }
    virtual void shutdown () { /* empty */ }
    virtual bool pump () = 0;
};
