#include "bytecode/instructions.hpp"

Inst_Copy_Const::Inst_Copy_Const (uint8_t reg, uint8_t value) {
	this->bytecode = BYTECODE_COPY_CONST;
	this->reg = reg;
	this->size = 1;

	this->data = (uint8_t*) malloc(1);
	*(this->data) = value;
}

Inst_Copy_Const::Inst_Copy_Const (uint8_t reg, uint16_t value) {
	this->bytecode = BYTECODE_COPY_CONST;
	this->reg = reg;
	this->size = 2;

	this->data = (uint8_t*) malloc(2);
	*(this->data + 0) = (value & 0x00FF) >> 0;
	*(this->data + 1) = (value & 0xFF00) >> 8;
}

Inst_Copy_Const::Inst_Copy_Const (uint8_t reg, uint32_t value) {
	this->bytecode = BYTECODE_COPY_CONST;
	this->reg = reg;
	this->size = 4;

	this->data = (uint8_t*) malloc(4);
	*(this->data + 0) = (value & 0x000000FF) >> 0;
	*(this->data + 1) = (value & 0x0000FF00) >> 8;
	*(this->data + 2) = (value & 0x00FF0000) >> 16;
	*(this->data + 3) = (value & 0xFF000000) >> 24;
}

Inst_Copy_Const::Inst_Copy_Const (uint8_t reg, uint64_t value) {
	this->bytecode = BYTECODE_COPY_CONST;
	this->reg = reg;
	this->size = 8;

	this->data = (uint8_t*) malloc(8);
	*(this->data + 0) = (value & 0x00000000000000FF) >> 0;
	*(this->data + 1) = (value & 0x000000000000FF00) >> 8;
	*(this->data + 2) = (value & 0x0000000000FF0000) >> 16;
	*(this->data + 3) = (value & 0x00000000FF000000) >> 24;
	*(this->data + 4) = (value & 0x000000FF00000000) >> 32;
	*(this->data + 5) = (value & 0x0000FF0000000000) >> 40;
	*(this->data + 6) = (value & 0x00FF000000000000) >> 48;
	*(this->data + 7) = (value & 0xFF00000000000000) >> 56;
}
