#pragma once

#include "steps/step.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/printer.hpp"

struct Print_Step : Step<>, Ast_Navigator {
    Print_Step () : Step("Ast Printer") { /* empty */ }

    void run (Ast_Statement* stm) {
        Ast_Printer::print(stm, 1);
    }
};
