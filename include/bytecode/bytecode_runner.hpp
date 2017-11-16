#pragma once

#include "parser/pipes.hpp"

#include "dyncall/dyncall.h"

#define BYTECODE_RUNNER_CALL_STACK_SIZE 64

struct Bytecode_Runner : Pipe {
    DCCallVM* vm = NULL;

    Bytecode_Runner (size_t vm_size = BYTECODE_RUNNER_CALL_STACK_SIZE) {
        this->vm = dcNewCallVM(vm_size);
    }

    ~Bytecode_Runner () {
        dcFree(this->vm);
    }

    void on_statement(Ast_Statement* stm);
};
