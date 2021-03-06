#pragma once

#include "service/modules.hpp"

#include "pipe.hpp"
#include "imp/parse_step.hpp"
#include "imp/import_modules.hpp"
#include "imp/resolve_idents.hpp"
#include "imp/type_check.hpp"
#include "imp/static_if.hpp"

#include "imp/generate_bytecode.hpp"

#include "imp/print_step.hpp"

/*
The build pipeline should handle the data from the "here's a file" command
to the file completed. The processing of each file is the following:

1. get the file's relative path
2. find absolute path from relative path
    - [error] file can't be found (print search paths)
3. load the contents of the whole file into memory
    - [error] file can't be read (readonly maybe)
4. parse the text of the file into an Ast_Scope
    - local identifier should be resolved here
    - [error] invalid syntax (lexer location)
5. handle import statements and remove them from scope
6. resolve identifiers to declarations (const only)
    - [error] multiple declarations with same name in scope (ast location)
    - [error] identifier's declaration can be found
7. infer types using existing data
    - [error] not enough data for type inference
    - if (!exp->inferred_type) do_work();
8. simplify constant values
    - propagate const values (functions, types & literals)
    - fold constants (2 + 3 -> 5)
    - replace regular if statements if possible
    - replace constant ifs by corresponding scope (then or else)
        - [error] constant if can't be resolved (ast location)
    - [if scope has new code] go back to step 5
9. check that all types are valid (using inferred type)
    - all types should be uniqued for further processing
    - [error] type mismatch on (ast location)
10. execute run directives & replace the Ast node by the return value
11. generate bytecode for each function
    - ignore internal and foreign functions
*/

#define BIND_PIPES(p1, p2) p1->output_queue = &p2->input_queue

struct Build_Pipeline {
    Parse_Step* parse_step = new Parse_Step();
    Import_Modules* import_modules = new Import_Modules();
    Resolve_Idents* resolve_idents = new Resolve_Idents();
    Static_If* static_if = new Static_If(&import_modules->input_queue);
    Type_Check* type_check = new Type_Check();

    Generate_Bytecode* generate_bytecode = new Generate_Bytecode();

    Print_Step* printer = new Print_Step();

    Array<Pipe*> pipes;

    void init (Build_Context* context) {
        pipes.push(this->parse_step);
        pipes.push(this->import_modules);
        pipes.push(this->resolve_idents);
        pipes.push(this->static_if);
        pipes.push(this->type_check);

        pipes.push(this->generate_bytecode);

        pipes.push(this->printer);

        BIND_PIPES(this->parse_step, this->import_modules);
        BIND_PIPES(this->import_modules, this->resolve_idents);
        BIND_PIPES(this->resolve_idents, this->static_if);
        BIND_PIPES(this->static_if, this->type_check);

        BIND_PIPES(this->type_check, this->generate_bytecode);

        //BIND_PIPES(this->generate_bytecode, this->printer);

        For (this->pipes) {
            it->context = context;
            it->init();
        }
    }

    void add_source_file (const char* absolute_path) {
        this->parse_step->push_in(Parse_Command(absolute_path));
    }

    void add_source_text (const char* text, size_t length) {
        this->parse_step->push_in(Parse_Command("(from text)", text, length));
    }

    bool pump () {
        bool has_work = false;
        For (this->pipes) {
            has_work |= it->pump();
        }
        return has_work;
    }

    void shutdown () {
        For (this->pipes) {
            it->shutdown();
        }
    }
};
