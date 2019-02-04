#pragma once

#include "steps/async_pipe.hpp"
#include "front/parser/parser.hpp"

#include "steps/imp/path_solver.hpp"

struct Parse_Step : Async_Pipe {
    Parser* parser = NULL;

    Parse_Step () : Async_Pipe("Parser") { /* empty */ }

    void setup () {
        auto c = this->context;
        auto internal_scope = new Internal_Scope(c->target_arch, c->target_os);
        this->parser = new Parser(internal_scope);
    }

    void handle (void* in) {
        auto source = reinterpret_cast<Code_Source*>(in);

        auto global_scope = this->parser->build_ast(source);

        for (auto stm : global_scope->statements) {
            this->pipe_out((void*) stm);
        }

        delete source;
    }
};
