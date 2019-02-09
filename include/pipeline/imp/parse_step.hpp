#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "front/parser/parser.hpp"
#include "front/parser/internal_scope.hpp"
#include "platform.hpp"

struct Parse_Command {
    const char* absolute_path;
    const char* source;
    size_t length;

    Parse_Command (const char* absolute_path, const char* source = NULL, size_t length = 0) {
        this->absolute_path = absolute_path;
        this->source = source;
        this->length = length;
    }
};

struct Parse_Step : Compiler_Pipe<Parse_Command, Ast_Scope*> {
    Modules* modules = NULL;

    Parse_Step (Modules* modules) : Compiler_Pipe("Parse") {
        this->modules = modules;
    }

    void handle (Parse_Command parse_command) {
        if (parse_command.source == NULL) {
            parse_command.source = os_read_full(parse_command.absolute_path, &parse_command.length);
        }

        auto file_scope = this->modules->get_file_scope(parse_command.absolute_path);
        this->context->parser->parse_into(file_scope, parse_command.absolute_path);

        this->push_out(file_scope);
    }
};
