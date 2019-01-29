#pragma once

#include "steps/sync_pipe.hpp"

#include "utils/ast_navigator.hpp"

struct Constant_If_Step : Sync_Pipe, Ast_Navigator {

    Constant_Propagation_Step() : Sync_Pipe("Constant If") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        Ast_Navigator::ast_handle(stm);
        this->pipe_out(in);
    }
};
