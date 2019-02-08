#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "front/parser/parser.hpp"
#include "front/parser/internal_scope.hpp"

#include "ast/printer.hpp"

struct Parse_Command {
    const char* absolute_path;
    const char* source;
    size_t length;

    Parse_Command (const char* source, size_t length, const char* absolute_path) {
        this->absolute_path = absolute_path;
        this->source = source;
        this->length = length;
    }
};

struct Parse_Step : Compiler_Pipe<Parse_Command, Ast_Scope*> {
    Ast_Printer* printer = new Ast_Printer();
    Parser* parser = NULL;

    Parse_Step () : Compiler_Pipe("Parse") {}

    void init () {
        auto internal_scope = new Internal_Scope(context->target_arch, context->target_os);
        this->parser = new Parser(internal_scope);
    }

    void handle (Parse_Command parse_command) {
        auto file_scope = this->parser->build_ast(parse_command.source,
            parse_command.length, parse_command.absolute_path);

        this->printer->print(file_scope);

        this->push_out(file_scope);
    }
};
