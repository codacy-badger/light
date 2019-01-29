#pragma once

#include "build_settings.hpp"
#include "utils/event_queue.hpp"
#include "step.hpp"

#include "imp/read_step.hpp"
#include "imp/parse_step.hpp"

#include "imp/print_step.hpp"

#include "imp/symbol_resolution_step.hpp"
#include "imp/constants/constant_solver_step.hpp"

#include <queue>
#include <vector>

struct Build_Steps {
    Build_Settings* settings = NULL;
    Event_Queue* events = NULL;

    Step<const char*>* read_step = NULL;
    Step<const char*, Ast_Statement*>* parse_step = NULL;
    std::vector<Step<>*> general_steps;

    Build_Steps (Event_Queue* events, Build_Settings* settings) {
        this->settings = settings;
        this->events = events;
    }

    void setup () {
        this->create_build_steps();
        this->connect_build_steps();
        this->setup_build_steps();

        for (auto input_file : this->settings->input_files) {
            this->read_step->input.push(input_file);
        }
    }

    void do_some_work () {
        bool has_worked = false;

        has_worked |= this->read_step->do_some_work();
        has_worked |= this->parse_step->do_some_work();
        for (auto step : this->general_steps) {
            has_worked |= step->do_some_work();
        }

        if (!has_worked) {
            this->events->push(new Compiler_Event(EVENT_COMPLETE));
        }
    }

    void create_build_steps () {
        this->read_step = new Read_Step();
        this->parse_step = new Parse_Step();
        this->general_steps.push_back(new Symbol_Resolution_Step());
        this->general_steps.push_back(new Constant_Solver_Step());

        this->general_steps.push_back(new Print_Step());

        this->read_step->events = this->events;
        this->parse_step->events = this->events;
        for (auto step : this->general_steps) {
            step->events = this->events;
        }
    }

    void connect_build_steps () {
        this->read_step->output = &this->parse_step->input;
        if (!this->general_steps.empty()) {
            auto first_step = this->general_steps[0];
            this->parse_step->output = &first_step->input;

            for (size_t i = 1; i < this->general_steps.size(); i++) {
                auto current_step = this->general_steps[i];
                auto last_step = this->general_steps[i - 1];

                last_step->output = &current_step->input;
            }
        }
    }

    void setup_build_steps () {
        this->read_step->setup(this->settings);
        this->parse_step->setup(this->settings);
        for (auto step : this->general_steps) {
            step->setup(this->settings);
        }
    }

    void teardown () {
        this->read_step->teardown();
        this->parse_step->teardown();
        for (auto step : this->general_steps) {
            step->teardown();
        }
    }
};
