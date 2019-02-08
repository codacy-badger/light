#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "front/parser/parser.hpp"
#include "front/parser/internal_scope.hpp"
#include "platform.hpp"

#include "ast/printer.hpp"

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
    Ast_Printer* printer = new Ast_Printer();

    Parse_Step () : Compiler_Pipe("Parse") {}

    void handle (Parse_Command parse_command) {
        if (parse_command.source == NULL) {
            parse_command.source = os_read_full(parse_command.absolute_path, &parse_command.length);
        }
        auto file_scope = this->context->parser->build_ast(parse_command.source,
            parse_command.length, parse_command.absolute_path);

        this->printer->print(file_scope);

        this->push_out(file_scope);
    }
};
