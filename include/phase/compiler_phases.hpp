#pragma once

#include "phase/async_phase.hpp"

#include "compiler_settings.hpp"

#include "imp/lexer/lexer.hpp"
#include "imp/parser/parser.hpp"
#include "imp/deps/module_cache.hpp"
#include "imp/deps/static_if.hpp"
#include "imp/deps/import_modules.hpp"
#include "imp/deps/symbol_resolution.hpp"
#include "imp/type_inference.hpp"
#include "imp/check/type_checking.hpp"
#include "imp/check/check_dependencies.hpp"

#include <vector>
#include <type_traits>

struct Compiler_Phases {
    Compiler_Settings* settings;
    std::vector<Phase*> phases;

    Compiler_Phases (Compiler_Settings* settings) {
        this->settings = settings;

        this->add_phase<Module_Cache>();

        this->add_phase<Lexer>();
        this->add_phase<Parser>();

        this->add_phase<Import_Modules>();
        this->add_phase<Symbol_Resolution>();
        this->add_phase<Type_Inference>();
        this->add_phase<Static_If>();

        this->add_phase<Check_Dependencies>();
        //this->add_phase<Type_Checking>();
    }

    template<typename T>
    void add_phase () {
        static_assert(std::is_base_of<Phase, T>::value, "Not a sub-type of Phase");
        static_assert(std::is_default_constructible<T>::value, "No default constructor");

        auto phase = new T();
        phase->event_to_id = CE_MODULE_READY;
        phase->settings = this->settings;
        phase->start();

        if (!this->phases.empty()) {
            this->phases.back()->event_to_id = phase->event_from_id;
        }

        this->phases.push_back(phase);
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
