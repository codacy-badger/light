#pragma once

#include "pipeline/compiler_pipe.hpp"

#include "ast/printer.hpp"

struct Print_Step : Compiler_Pipe<Ast_Statement*> {
    Print_Step () : Compiler_Pipe("Ast Printer") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        this->context->printer->print(global_statement);
        this->push_out(global_statement);
    }
};
