#pragma once

#include "steps/multi_pipe.hpp"

#include "constant_propagation_step.hpp"
#include "constant_folding_step.hpp"

struct Constant_Solver_Step : Multi_Pipe {
    Constant_Solver_Step() : Multi_Pipe() { /* empty */ }

    void build_sub_pipes () {
        this->sub_pipes.push_back(new Constant_Propagation_Step());
        this->sub_pipes.push_back(new Constant_Folding_Step());
    }
};
