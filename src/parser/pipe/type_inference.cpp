#pragma once

#include "parser/pipe/type_inference.hpp"

void Type_Inference::on_block_begin(Ast_Block* block) {
}

void Type_Inference::on_statement(Ast_Statement* stm) {
    this->to_next(stm);
}

void Type_Inference::on_block_end(Ast_Block* block) {
}

void Type_Inference::on_finish () {
    this->try_finish();
}
