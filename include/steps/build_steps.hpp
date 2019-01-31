#pragma once

#include "imp/path_solver.hpp"
#include "imp/file_cache.hpp"

#include "imp/read_step.hpp"
#include "imp/parse_step.hpp"

#include "imp/external_dependencies.hpp"
#include "imp/symbol_resolution_step.hpp"
#include "imp/constants/constant_solver_step.hpp"

#include "imp/print_step.hpp"

struct Build_Steps : Multi_Pipe {

    void build_sub_pipes (Build_Settings*) {
        this->add(new Path_Solver());
        this->add(new File_Cache());

        this->add(new Read_Step());
        this->add(new Parse_Step());

        this->add(new External_Dependencies(this));
        this->add(new Symbol_Resolution_Step());
        //this->add(new Constant_Solver_Step());

        this->add(new Print_Step());
    }
};
