#pragma once

#include <stdint.h>

#include "compiler.hpp"

struct Bytecode_Interpreter;

enum Inst_Bytecode : uint8_t {
	BYTECODE_NOOP,

	BYTECODE_COPY_CONST,
	BYTECODE_COPY_REG,

	BYTECODE_STACK_ALLOCA,
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
