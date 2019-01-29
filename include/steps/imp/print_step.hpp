#pragma once

#include "steps/sync_pipe.hpp"

#include "ast/printer.hpp"

struct Print_Step : Sync_Pipe {
    Ast_Printer* printer = new Ast_Printer();

    Print_Step () : Sync_Pipe("Ast Printer") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        printf("\n");
        this->printer->print(stm);
        printf("\n");
        this->pipe_out((void*) stm);
    }
};
