#pragma once

#include "steps/sync_pipe.hpp"
#include "front/parser/parser.hpp"

struct Parse_Step : Sync_Pipe {
    Parser* parser = NULL;

    Parse_Step () : Sync_Pipe("Parser") { /* empty */ }

    void setup (Build_Settings* s) {
        auto internal_scope = new Internal_Scope(s->target_arch, s->target_os);
        this->parser = new Parser(internal_scope);
    }

    void handle (void* in) {
        auto source_code = static_cast<const char*>(in);

        auto global_scope = this->parser->build_ast(source_code);
        this->pipe_out((void*) global_scope);
    }
};
