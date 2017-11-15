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
	BYTECODE_STACK_ALLOCATE,
	BYTECODE_STACK_OFFSET,

	BYTECODE_LOAD,
	BYTECODE_STORE,

	BYTECODE_NOT,
	BYTECODE_NEG,

	BYTECODE_ADD,
	BYTECODE_SUB,
	BYTECODE_MUL,
	BYTECODE_DIV,

	BYTECODE_CALL,
};

struct Instruction {
	uint8_t bytecode = BYTECODE_NOOP;
	const char* filename = NULL;
	size_t line = 0;
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

struct Inst_Global_Offset : Instruction {
	uint8_t reg = 0;
	uint8_t size = 0;

	Inst_Global_Offset (uint8_t reg, uint8_t size) {
		this->bytecode = BYTECODE_GLOBAL_OFFSET;
		this->reg = reg;
		this->size = size;
	}
};

struct Inst_Stack_Allocate : Instruction {
	uint8_t size = 0;

	Inst_Stack_Allocate (uint8_t size) {
		this->bytecode = BYTECODE_STACK_ALLOCATE;
		this->size = size;
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

struct Inst_Load : Instruction {
	uint8_t dest = 0;
	uint8_t src = 0;
	uint8_t size = 0;

	Inst_Load (uint8_t dest, uint8_t src, uint8_t size) {
		this->bytecode = BYTECODE_LOAD;
		this->dest = dest;
		this->src = src;
		this->size = size;
	}
};

struct Inst_Store : Instruction {
	uint8_t dest = 0;
	uint8_t src = 0;
	uint8_t size = 0;

	Inst_Store (uint8_t dest, uint8_t src, uint8_t size) {
		this->bytecode = BYTECODE_STORE;
		this->dest = dest;
		this->src = src;
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

struct Inst_Sub : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Sub (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_SUB;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};

struct Inst_Mul : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Mul (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_MUL;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};

struct Inst_Div : Instruction {
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;

	Inst_Div (uint8_t reg1, uint8_t reg2) {
		this->bytecode = BYTECODE_DIV;
		this->reg1 = reg1;
		this->reg2 = reg2;
	}
};

const uint8_t BYTECODE_CC_UNDEFINED = 0x0;
const uint8_t BYTECODE_CC_CDECL = 0x1;
const uint8_t BYTECODE_CC_STDCALL = 0x2;
const uint8_t BYTECODE_CC_FASTCALL = 0x3;

struct Inst_Call : Instruction {
	uint8_t reg = 0;
	uint8_t calling_convention = BYTECODE_CC_UNDEFINED;
	uint8_t param_count = 0;
	uint8_t* data = NULL;

	Inst_Call (uint8_t reg, Inst_Call_Convention calling_convention) {
		this->bytecode = BYTECODE_CALL;
		this->calling_convention = calling_convention;
		this->reg = reg;
	}
};
