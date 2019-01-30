#pragma once

#include "steps/sync_pipe.hpp"

#include "steps/imp/path_solver.hpp"

struct External_Dependencies : Sync_Pipe {
    Pipe* starting_pipe = NULL;

    External_Dependencies(Pipe* starting_pipe) : Sync_Pipe("External Dependencies") {
        this->starting_pipe = starting_pipe;
    }

    void handle (void* in) {
        auto stm = reinterpret_cast<Ast_Statement*>(in);

        if (stm->stm_type == AST_STATEMENT_IMPORT) {
            auto import = static_cast<Ast_Import*>(stm);

            auto source = new Code_Source(import->path, import->location.filename);
            this->starting_pipe->pipe_in((void*) source);
            return;
        }

        // TODO: handle foreign function declarations

        this->pipe_out(in);
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
