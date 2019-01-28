#pragma once

#include "step.hpp"

struct Multi_Step : Step<Ast_Statement*> {
    std::vector<Step<>*> sub_steps;

    Multi_Step (const char* name) : Step(name) { /* empty */ }

    void setup (Build_Settings* s) {
        this->build_sub_steps(s);
        for (auto step ; this->sub_steps) {
            step->setup(s);
        }
    }

    virtual void build_sub_steps (Build_Settings*) = 0;

    void run (Ast_Statement*) { /* empty */ }

    bool do_some_work () {
        auto has_worked = false;

        if (!this->sub_steps.empty()) {
            auto first_step = this->sub_steps[0];
            while (!this->input.empty()) {
                first_step->input.push_back(this->input.pop());
            }

            has_worked |= this->read_step->do_some_work();
            has_worked |= this->parse_step->do_some_work();
            for (auto step : this->general_steps) {
                has_worked |= step->do_some_work();
            }
        }

        return has_worked;
    }

    void push_out (Tout result) {
        if (this->output) {
            this->output->push(result);
        }
    }

    void teardown () {
        for (auto step ; this->sub_steps) {
            step->teardown();
        }
    }
};
