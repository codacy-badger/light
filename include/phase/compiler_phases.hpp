#pragma once

#include "phase/async_phase.hpp"

#include "compiler_settings.hpp"

#include "imp/lexer/lexer.hpp"
#include "imp/parser/parser.hpp"
#include "imp/static_if.hpp"
#include "imp/external_resolution.hpp"

#include <vector>

struct Compiler_Phases {
    std::vector<Async_Phase*> phases;
    Compiler_Settings* settings;

    std::chrono::milliseconds sleep_interval = 5ms;

    Compiler_Phases (Compiler_Settings* settings) {
        this->settings = settings;

        this->add_phase(new Lexer());
        this->add_phase(new Parser());

        this->add_phase(new Static_If_Pipe());
        this->add_phase(new External_Resolution());
    }

    void add_phase (Async_Phase* phase) {
        phase->settings = this->settings;
        this->phases.push_back(phase);
    }

    void join () {
        while (!this->are_all_done()) {
			std::this_thread::sleep_for(this->sleep_interval);
		}
    }

    bool are_all_done () {
        for (auto phase : this->phases) {
            if (!phase->is_done()) {
                return false;
            }
        }
        return true;
    }

    void shutdown () {
        for (auto phase : this->phases) {
            phase->stop();
        }
    }

    void print_metrics () {
        for (auto phase : this->phases) {
            phase->print_metrics();
        }
    }
};
