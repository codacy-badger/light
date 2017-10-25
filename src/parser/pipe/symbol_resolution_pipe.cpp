#pragma once

#include "parser/pipe/symbol_resolution_pipe.hpp"

#include <vector>

#include "compiler.hpp"

void Symbol_Resolution::onStatement(Ast_Statement* stm) {
    this->toNext(stm);
}

void Symbol_Resolution::onFinish () {
    this->tryFinish();
}
