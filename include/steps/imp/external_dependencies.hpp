#pragma once

#include "steps/sync_pipe.hpp"
#include "utils/ast_navigator.hpp"

#include "steps/imp/path_solver.hpp"

struct External_Dependencies : Sync_Pipe, Ast_Navigator {
    Pipe* starting_pipe = NULL;

    External_Dependencies(Pipe* starting_pipe) : Sync_Pipe("External Dependencies") {
        this->starting_pipe = starting_pipe;
    }

    void handle (void* in) {
        auto stm = reinterpret_cast<Ast_Statement*>(in);

        Ast_Navigator::ast_handle(stm);
        this->pipe_out(in);
    }

    void ast_handle (Ast_Import* import) {
        this->remove_current_statement = true;

        auto source = new Code_Source(import->path, import->location.filename);
        this->starting_pipe->pipe_in((void*) source);
    }

    /*void ast_handle (Ast_Foreign* foreign) {
        this->remove_current_statement = true;

        auto new_stms = &foreign->declarations;
        auto stms = &this->current_scope->statements;
        auto foreign_stm = std::find(stms->begin(), stms->end(), foreign);
        stms->insert(foreign_stm + 1, new_stms->begin(), new_stms->end());

        this->foreign_functions += new_stms->size();
    }*/
};
