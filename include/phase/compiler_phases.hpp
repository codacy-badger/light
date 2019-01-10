#pragma once

#include "phase/async_phase.hpp"

#include "compiler_settings.hpp"

#include "imp/lexer/lexer.hpp"
#include "imp/parser/parser.hpp"
#include "imp/deps/module_cache.hpp"
#include "imp/deps/static_if.hpp"
#include "imp/deps/local_symbol_resolution.hpp"
#include "imp/deps/external_modules.hpp"
#include "imp/deps/external_symbol_resolution.hpp"
#include "imp/check/check_dependencies.hpp"

#include <vector>

struct Compiler_Phases {
    std::vector<Phase*> phases;
    Compiler_Settings* settings;

    std::chrono::milliseconds sleep_interval = 5ms;

    Compiler_Phases (Compiler_Settings* settings) {
        this->settings = settings;

        this->add_phase(new Module_Cache());

        this->add_phase(new Lexer());
        this->add_phase(new Parser());

        this->add_phase(new Local_Symbol_Resolution());
        this->add_phase(new External_Modules());
        this->add_phase(new External_Symbol_Resolution());
        this->add_phase(new Static_If());

        this->add_phase(new Check_Dependencies());
    }

    void add_phase (Phase* phase) {
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
