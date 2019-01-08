#pragma once

#include "scoped_pipe.hpp"

struct Scoped_Statement_Pipe : Scoped_Pipe {
    Ast_Statement* current_stm = NULL;

    void handle (Ast_Statement** stm_ptr) {
        this->current_stm = (*stm_ptr);
        Pipe::handle(stm_ptr);
    }

    vector<Ast_Statement*>::iterator get_current_stm_location () {
        return find(this->current_scope->statements.begin(),
            this->current_scope->statements.end(), this->current_stm);
    }

    vector<Ast_Statement*>::iterator delete_current_statement () {
        auto location = this->get_current_stm_location();
		return this->current_scope->statements.erase(location);
    }
};
