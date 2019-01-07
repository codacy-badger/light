#pragma once

#include "phase/async_phase.hpp"

#include "compiler_settings.hpp"

#include "external_resolution.hpp"

#include <vector>

struct Compiler_Phases {

    std::vector<Async_Phase*> phases;

    Compiler_Phases (Compiler_Settings*) {
        this->add_phase(new External_Resolution());
    }

    void add_phase (Async_Phase* phase) {
        this->phases.push_back(phase);
    }

    void shutdown () {
        for (auto phase : this->phases) {
            phase->stop();
        }
    }
};
