#pragma once

#include "steps/step.hpp"

#include "ast/printer.hpp"

struct Print_Step : Step<> {
    Ast_Printer* printer = new Ast_Printer();

    Print_Step () : Step("Ast Printer") { /* empty */ }

    void run (Ast_Statement* stm) {
        printf("\n");
        this->printer->print(stm);
        printf("\n");
    }
};
