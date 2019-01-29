#pragma once

#include "steps/step.hpp"

#include "utils/ast_navigator.hpp"

struct Constant_If_Step : Step<>, Ast_Navigator {
    bool* has_worked;

    Constant_Propagation_Step(bool* has_worked) : Step("Constant If") {
        this->has_worked = has_worked;
    }

    void run (Ast_Statement* stm) {
        Ast_Navigator::ast_handle(stm);
    }
};
