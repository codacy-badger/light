#pragma once

#include "multi_pipe.hpp"

#include "imp/parse_step.hpp"
#include "imp/symbol_resolution_step.hpp"
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

struct Build_Pipeline : Multi_Pipe {

    void build_sub_pipes () {
        this->add(new Parse_Step());

        this->add(new Symbol_Resolution_Step());

        this->add(new Print_Step());
    }
};
