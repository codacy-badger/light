#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "parser/parser.hpp"
#include "parser/internal_scope.hpp"
#include "platform.hpp"

#include "pipeline/service/modules.hpp"

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

struct Parse_Step : Compiler_Pipe<Parse_Command, Ast_Statement*> {
    Parse_Step () : Compiler_Pipe("Parse") { /* empty */ }

    void handle (Parse_Command parse_command) {
        if (parse_command.source == NULL) {
            parse_command.source = os_read_entire_file(parse_command.absolute_path, &parse_command.length);
        }

        auto file_scope = this->context->modules->get_file_scope(parse_command.absolute_path);
        this->context->parser->parse_into(file_scope, parse_command.absolute_path);
        file_scope->scope_flags |= SCOPE_FLAG_FULLY_PARSED;

        For (file_scope->statements) {
            this->push_out(it);
        }
    }
};
