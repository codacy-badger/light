#pragma once

#include "async_phase.hpp"

#include "module.hpp"
#include "compiler_events.hpp"

struct External_Resolution : Async_Phase {

    External_Resolution() : Async_Phase(CE_MODULE_PARSED) { /* empty */ }

    void handle (void* data) {
        auto module = reinterpret_cast<Module*>(data);

        printf("Resolving external dependencies for '%s'\n", module->absolute_path);
    }
};
