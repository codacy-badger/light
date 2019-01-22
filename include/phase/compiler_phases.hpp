#pragma once

#include "compiler_settings.hpp"
#include "ast/type_table.hpp"
#include "bytecode/interpreter.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"

#include "imp/parser/parser.hpp"
#include "imp/deps/module_cache.hpp"
#include "imp/deps/static_if.hpp"
#include "imp/deps/import_modules.hpp"
#include "imp/deps/symbol_resolution.hpp"
#include "imp/type_inference.hpp"
#include "imp/type_conversion.hpp"
#include "imp/unique_types.hpp"
#include "imp/check/type_checking.hpp"
#include "imp/check/check_dependencies.hpp"
#include "imp/array_attributes.hpp"
#include "imp/bytecode/register_allocator.hpp"
#include "imp/bytecode/generate_bytecode.hpp"
#include "imp/bytecode/run_bytecode.hpp"

#include <vector>
#include <type_traits>

struct Compiler_Phases {
    Compiler_Settings* settings;

    Interpreter* interpreter = new Interpreter();
    Type_Table* type_table = new Type_Table();

    std::vector<Phase*> phases;

    Compiler_Phases (Compiler_Settings* settings) {
        this->settings = settings;

        this->add_phase<Module_Cache>();
        this->add_phase<Parser>();

        this->add_phase<Import_Modules>();
        this->add_phase<Symbol_Resolution>();
        this->add_phase<Type_Inference>();
        this->add_phase<Array_Attributes>();
        this->add_phase<Static_If>();

        this->add_phase<Check_Dependencies>();
        this->add_phase<Unique_Types>(this->type_table);
        this->add_phase<Type_Conversion>();
        this->add_phase<Type_Checking>();

        this->add_phase<Register_Allocator>(this->interpreter);
        this->add_phase<Generate_Bytecode>(this->interpreter->constants, this->interpreter->globals);
        this->add_phase<Run_Bytecode>(this->interpreter);
    }

    template<typename T, typename... Args>
    void add_phase (Args&&... args) {
        static_assert(std::is_base_of<Phase, T>::value, "Not a sub-type of Phase");

        auto phase = new T(std::forward<Args>(args)...);
        phase->event_to_id = CE_MODULE_READY;
        phase->start(this->settings);

        if (!this->phases.empty()) {
            this->phases.back()->event_to_id = phase->event_from_id;
        }

        this->phases.push_back(phase);
    }

    void handle_sync_events () {
        for (auto phase : this->phases) {
            if (!phase->is_async) {
                phase->process_events();
            }
        }
    }

    bool all_done () {
        bool all_done = true;
        for (auto phase : this->phases) {
            auto is_done = phase->is_done();
            all_done &= is_done;
        }
        return all_done;
    }

    void shutdown () {
        for (auto phase : this->phases) {
            phase->stop();
        }
    }

    void print_metrics () {
        if (Logger::is_debug()) {
            Logger::debug("Printing compiler phases metrics:");
            for (auto phase : this->phases) {
                phase->print_metrics();
            }
        }
    }
};
