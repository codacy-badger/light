#pragma once

#include "steps/step.hpp"
#include "front/parser/parser.hpp"

struct Parse_Step : Step<const char*, Ast_Statement*> {
    Parser* parser = NULL;

    Parse_Step () : Step("Parser") { /* empty */ }

    void setup (Build_Settings* s) {
        auto internal_scope = new Internal_Scope(s->target_arch, s->target_os);
        this->parser = new Parser(internal_scope);
    }

    void run (const char* source_code) {
        auto global_scope = this->parser->build_ast(source_code);

        for (auto stm : global_scope->statements) {
            this->push_out(stm);
        }
    }
};
