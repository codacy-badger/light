#pragma once

#include "imp/read_step.hpp"
#include "imp/parse_step.hpp"

#include "imp/print_step.hpp"

#include "imp/symbol_resolution_step.hpp"
#include "imp/constants/constant_solver_step.hpp"

struct Build_Steps : Multi_Pipe {

    void build_sub_pipes () {
        this->add(new Read_Step());
        this->add(new Parse_Step());
        this->add(new Symbol_Resolution_Step());
        this->add(new Constant_Solver_Step());
        this->add(new Print_Step());
    }
};
