#pragma once

#include "imp/file_finder.hpp"
#include "imp/scope_cache.hpp"

#include "pipe.hpp"
#include "imp/read_file_step.hpp"
#include "imp/parse_step.hpp"
#include "imp/module_dependencies.hpp"

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
    File_Finder* file_finder = new File_Finder();
    Scope_Cache* scope_cache = new Scope_Cache();

    Read_File_Step* read_file_step = new Read_File_Step();
    Parse_Step* parse_step = new Parse_Step();
    Module_Dependencies* module_dependencies = new Module_Dependencies();

    std::vector<Pipe*> pipes;

    void init (Build_Context* context) {
        this->file_finder->init(context);

        pipes.push_back(this->read_file_step);
        pipes.push_back(this->parse_step);
        pipes.push_back(this->module_dependencies);

        BIND_PIPES(this->read_file_step, this->parse_step);
        BIND_PIPES(this->parse_step, this->module_dependencies);

        for (auto pipe : this->pipes) {
            pipe->context = context;
            pipe->init();
        }
    }

    void add_source_file (const char* absolute_path) {
        this->read_file_step->push_in(Source_File(absolute_path));
    }

    void add_source_text (const char*, size_t) { }

    bool pump () {
        bool has_work = false;
        for (auto pipe : this->pipes) {
            has_work |= pipe->pump();
        }
        return has_work;
    }

    void shutdown () {
        for (auto pipe : this->pipes) {
            pipe->shutdown();
        }
    }
};
