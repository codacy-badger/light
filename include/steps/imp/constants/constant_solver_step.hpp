#pragma once

#include "steps/multi_step.hpp"

#include "constant_propagation_step.hpp"
#include "constant_folding_step.hpp"

struct Constant_Solver_Step : Multi_Step {
    bool has_worked = false;

    Constant_Solver_Step() : Multi_Step("Constant Solver") { /* empty */ }

    void build_sub_steps (Build_Settings*) {
        this->sub_steps.push_back(new Constant_Propagation_Step(&has_worked));
        this->sub_steps.push_back(new Constant_Folding_Step(&has_worked));
    }

    void run (Ast_Statement* stm) {
        has_worked = false;
        this->run_steps(stm);
        while (has_worked) {
            has_worked = false;
            this->run_steps(stm);
        }
        this->push_out(stm);
    }
};
