#pragma once

#include "phase/pipeline/scoped_pipe.hpp"

#include "compiler.hpp"

struct Run_Directive : Scoped_Pipe {
    Interpreter* interp;

    Run_Directive (Interpreter* interp) {
        this->pipe_name = "Run_Directive";
        this->interp = interp;
    }

    void handle (Ast_Directive_Run** run_ptr) {
        auto run = (*run_ptr);

        this->interp->run(&run->bytecode);

        if (run->inferred_type != Types::type_void) {
            auto output = new Ast_Literal();
            output->inferred_type = run->inferred_type;

            auto reg = this->interp->registers[run->expression->reg];
            memcpy(&output->int_value, reg, INTERP_REGISTER_SIZE);

            DEBUG(run, "Run directive output: %d (%lld)\n",
                run->expression->reg, output->uint_value);

            (*run_ptr) = reinterpret_cast<Ast_Directive_Run*>(output);
        }
    }
};
