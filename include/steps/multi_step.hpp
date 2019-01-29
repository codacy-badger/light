#pragma once

#include "step.hpp"

struct Multi_Step : Step<Ast_Statement*> {
    std::vector<Step<>*> sub_steps;

    Multi_Step (const char* name) : Step(name) { /* empty */ }

    void setup (Build_Settings* s) {
        this->build_sub_steps(s);
        for (auto step : this->sub_steps) {
            step->setup(s);
        }
    }

    virtual void build_sub_steps (Build_Settings*) = 0;

    virtual void run (Ast_Statement* stm) {
        this->run_steps(stm);
        this->push_out(stm);
    }

    void run_steps (Ast_Statement* stm) {
        for (auto step : this->sub_steps) {
            step->run(stm);
        }
    }

    bool do_some_work () {
        auto has_worked = false;

        if (!this->sub_steps.empty()) {
            while (!this->input.empty()) {
                has_worked = true;

                auto stm = this->input.pop();
                this->run(stm);
            }
        }

        return has_worked;
    }

    void teardown () {
        for (auto step : this->sub_steps) {
            step->teardown();
        }
    }
};
