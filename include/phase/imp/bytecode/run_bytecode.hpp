#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"
#include "ast/ast_factory.hpp"

#include "bytecode/instructions.hpp"
#include "bytecode/interpreter.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"

#include "util/logger.hpp"

struct Run_Bytecode : Phase, Ast_Navigator {

    Run_Bytecode () : Phase("Run Bytecode", CE_BYTECODE_RUN, true) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

    void ast_handle (Ast_Run*) {
        //Logger::debug(run, "About to run bytecode (%zd)", run->bytecode.size());
    }
};
