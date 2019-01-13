#pragma once

#include "phase/async_phase.hpp"

#include "compiler_settings.hpp"

#include "imp/lexer/lexer.hpp"
#include "imp/parser/parser.hpp"
#include "imp/deps/module_cache.hpp"
#include "imp/deps/static_if.hpp"
#include "imp/deps/import_modules.hpp"
#include "imp/deps/symbol_resolution.hpp"
#include "imp/check/check_dependencies.hpp"

#include <vector>
#include <type_traits>

struct Compiler_Phases {
    std::vector<Phase*> phases;
    Compiler_Settings* settings;

    std::chrono::milliseconds sleep_interval = 1ms;

    Compiler_Phases (Compiler_Settings* settings) {
        this->settings = settings;

        this->add_phase<Module_Cache>(CE_IMPORT_MODULE, CE_MODULE_RUN_LEXER);

        this->add_phase<Lexer>(CE_MODULE_RUN_PARSER);
        this->add_phase<Parser>(CE_MODULE_RESOLVE_IMPORTS);

        this->add_phase<Import_Modules>(CE_MODULE_RESOLVE_EXTERNAL_SYMBOLS);
        this->add_phase<Symbol_Resolution>(CE_MODULE_RESOLVE_IFS);
        this->add_phase<Static_If>(CE_MODULE_CHECK_DEPENDENCIES);

        this->add_phase<Check_Dependencies>();
    }

    template<typename T>
    void add_phase (size_t event_from_id, size_t event_to_id) {
        static_assert(std::is_base_of<Phase, T>::value, "Not a sub-type of Phase");
        static_assert(std::is_default_constructible<T>::value, "No default constructor");

        auto phase = new T();
        phase->event_from_id = event_from_id;
        phase->event_to_id = event_to_id;
        phase->settings = this->settings;
        phase->start();

        this->phases.push_back(phase);
    }

    template<typename T>
    void add_phase (size_t event_to_id = CE_MODULE_READY) {
        this->add_phase<T>(this->get_last_event(), event_to_id);
    }

    size_t get_last_event () {
        return this->phases.back()->event_to_id;
    }

    void join () {
        while (!this->are_all_done());
    }

    bool are_all_done () {
        bool all_done = true;
        for (auto phase : this->phases) {
            all_done &= phase->is_done();
        }
        return all_done;
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
