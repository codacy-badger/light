#pragma once

#include <stdint.h>
#include <malloc.h>
#include <string.h>

struct Bytecode_Interpreter;

enum Inst_Bytecode : uint8_t {
	BYTECODE_NOOP,

	BYTECODE_COPY,

	BYTECODE_SET_INTEGER,
	BYTECODE_SET_DECIMAL,

	BYTECODE_GLOBAL_OFFSET,
	BYTECODE_STACK_OFFSET,

	BYTECODE_DEREFERENCE,
	BYTECODE_STORE,

	BYTECODE_NOT,
	BYTECODE_NEG,

	BYTECODE_ADD,
	BYTECODE_SUB,
	BYTECODE_MUL,
	BYTECODE_DIV,
};

struct Instruction {
	uint8_t bytecode = BYTECODE_NOOP;
	const char* filename = NULL;
	size_t line = 0;
};

struct Inst_Set_Integer : Instruction {
	uint8_t reg = 0;
	uint8_t size = 0;
	uint8_t* data = NULL;

	template<typename N>
	Inst_Set_Integer (uint8_t reg, N value)
		: Inst_Set_Integer(reg, sizeof(N), reinterpret_cast<uint8_t*>(&value)) {}
	Inst_Set_Integer (uint8_t reg, uint8_t size, uint8_t* data) {
		this->bytecode = BYTECODE_SET_INTEGER;
		this->reg = reg;
		this->size = size;
		this->data = (uint8_t*) malloc(size);
		memcpy(this->data, data, size);
	}
};

struct Inst_Copy : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Copy (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_COPY;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};

struct Inst_Stack_Offset : Instruction {
	uint8_t reg = 0;
	uint8_t size = 0;

	Inst_Stack_Offset (uint8_t reg, uint8_t size) {
		this->bytecode = BYTECODE_STACK_OFFSET;
		this->reg = reg;
		this->size = size;
	}
};

struct Inst_Add : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Add (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_ADD;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};
