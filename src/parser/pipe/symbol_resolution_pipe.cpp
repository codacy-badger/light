#pragma once

#include "parser/pipe/symbol_resolution_pipe.hpp"

#include <vector>

#include "compiler.hpp"

void Symbol_Resolution::on_block_begin(Ast_Block* block) {
}

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    this->to_next(stm);
}

void Symbol_Resolution::on_block_end(Ast_Block* block) {
}

void Symbol_Resolution::on_finish () {
    this->try_finish();
}
