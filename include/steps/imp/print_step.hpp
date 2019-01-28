#pragma once

#include "steps/step.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/printer.hpp"

struct Print_Step : Step<>, Ast_Navigator {
    Ast_Printer* printer = new Ast_Printer();

    Print_Step () : Step("Ast Printer") { /* empty */ }

    void run (Ast_Statement* stm) {
        printf("\n");
        this->printer->print(stm);
        printf("\n");
    }
};
