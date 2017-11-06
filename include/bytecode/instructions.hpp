#pragma once

#include <stdint.h>

#include "compiler.hpp"

struct Bytecode_Interpreter;

enum Inst_Bytecode : uint8_t {
	BYTECODE_NOOP,

	BYTECODE_COPY_CONST,
	BYTECODE_COPY_REG,

	BYTECODE_STACK_ALLOCA,

	BYTECODE_ADD_I32,
	BYTECODE_ADD_REG,
};

struct Instruction {
	uint8_t bytecode = BYTECODE_NOOP;
};

struct Inst_Noop : Instruction {
	Inst_Noop () { this->bytecode = BYTECODE_NOOP; }
};

struct Inst_Copy_Const : Instruction {
	uint8_t reg = 0;
	uint8_t size = 0;
	uint8_t* data = NULL;

	Inst_Copy_Const (uint8_t reg, uint8_t value);
	Inst_Copy_Const (uint8_t reg, uint16_t value);
	Inst_Copy_Const (uint8_t reg, uint32_t value);
	Inst_Copy_Const (uint8_t reg, uint64_t value);
	Inst_Copy_Const (uint8_t reg, uint8_t size, uint8_t* data) {
		this->bytecode = BYTECODE_COPY_CONST;
		this->reg = reg;
		this->size = size;
		this->data = data;
	}
};

struct Inst_Copy_Reg : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Copy_Reg (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_COPY_REG;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};

struct Inst_Stack_Alloca : Instruction {
	uint8_t reg = 0;
	uint8_t size = 0;

	Inst_Stack_Alloca (uint8_t reg, uint8_t size) {
		this->bytecode = BYTECODE_STACK_ALLOCA;
		this->reg = reg;
		this->size = size;
	}
};

struct Inst_Add_I32 : Instruction {
	uint8_t reg = 0;
	uint8_t* data = NULL;

	Inst_Add_I32 (uint8_t reg, uint32_t value) {
		this->bytecode = BYTECODE_ADD_I32;
		this->reg = reg;

		this->data = (uint8_t*) malloc(4);
		*(this->data + 0) = (value & 0x000000FF) >> 0;
		*(this->data + 1) = (value & 0x0000FF00) >> 8;
		*(this->data + 2) = (value & 0x00FF0000) >> 16;
		*(this->data + 3) = (value & 0xFF000000) >> 24;
	}
};

struct Inst_Add_Reg : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Add_Reg (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_ADD_REG;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};
