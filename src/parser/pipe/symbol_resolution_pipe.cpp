#pragma once

#include "parser/pipe/symbol_resolution_pipe.hpp"

#include <vector>

#include "compiler.hpp"

Symbol_Resolution::Symbol_Resolution () {
    this->cache["void"] =   Light_Compiler::type_def_void;
    this->cache["i1"]   =   Light_Compiler::type_def_i1;
    this->cache["i32"]  =   Light_Compiler::type_def_i32;
}

void Symbol_Resolution::onStatement(Ast_Statement* stm) {
    this->toNext(stm);
}

void Symbol_Resolution::onFinish () {
    this->tryFinish();
}
