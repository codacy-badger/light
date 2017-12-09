#pragma once

#include <stdint.h>
#include <malloc.h>
#include <string.h>

#include "parser/ast.hpp"

struct Bytecode_Interpreter;

enum Inst_Bytecode : uint8_t {
	BYTECODE_NOOP,

	BYTECODE_COPY,
	BYTECODE_CAST,

	BYTECODE_SET,

	BYTECODE_CONSTANT_OFFSET,
	BYTECODE_GLOBAL_OFFSET,
	BYTECODE_STACK_ALLOCATE,
	BYTECODE_STACK_FREE,
	BYTECODE_STACK_OFFSET,

	BYTECODE_LOAD,
	BYTECODE_STORE,

	BYTECODE_UNARY,
	BYTECODE_BINARY,

	BYTECODE_JUMP,
	BYTECODE_JUMP_IF_FALSE,

	BYTECODE_CALL_SETUP,
	BYTECODE_CALL_PARAM,
	BYTECODE_CALL,
	BYTECODE_RETURN,
};

enum Inst_Unop : uint8_t {
	BYTECODE_ARITHMETIC_NEGATE,
	BYTECODE_LOGICAL_NEGATE,
	BYTECODE_BITWISE_NEGATE,
};

enum Inst_Binop : uint8_t {
	BYTECODE_LOGICAL_AND,
	BYTECODE_LOGICAL_OR,

	BYTECODE_ADD,
	BYTECODE_SUB,
	BYTECODE_MUL,
	BYTECODE_DIV,

	BYTECODE_BITWISE_AND,
	BYTECODE_BITWISE_OR,
	BYTECODE_BITWISE_XOR,
	BYTECODE_BITWISE_RIGHT_SHIFT,
	BYTECODE_BITWISE_LEFT_SHIFT,

	BYTECODE_EQ,
	BYTECODE_NEQ,
	BYTECODE_LT,
	BYTECODE_LTE,
	BYTECODE_GT,
	BYTECODE_GTE,
};

const uint8_t BYTECODE_TYPE_VOID 	= 0x00;
const uint8_t BYTECODE_TYPE_BOOL	= 0x01;
const uint8_t BYTECODE_TYPE_S8 		= 0x02;
const uint8_t BYTECODE_TYPE_S16 	= 0x03;
const uint8_t BYTECODE_TYPE_S32 	= 0x04;
const uint8_t BYTECODE_TYPE_S64 	= 0x05;
const uint8_t BYTECODE_TYPE_U8 		= 0x06;
const uint8_t BYTECODE_TYPE_U16 	= 0x07;
const uint8_t BYTECODE_TYPE_U32 	= 0x08;
const uint8_t BYTECODE_TYPE_U64 	= 0x09;
const uint8_t BYTECODE_TYPE_F32 	= 0x0A;
const uint8_t BYTECODE_TYPE_F64 	= 0x0B;
const uint8_t BYTECODE_TYPE_POINTER	= 0x0C;

uint8_t bytecode_get_type (Ast_Type_Definition* decl_ty);
uint8_t bytecode_get_type (Ast_Expression* exp);
size_t bytecode_get_size (uint8_t bytecode_type);
bool bytecode_has_sign (uint8_t bytecode_type);
uint8_t bytecode_unsigned_to_signed (uint8_t bytecode_type);

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

struct Inst_Cast : Instruction {
	uint8_t reg = 0;
	uint8_t type_from = 0;
	uint8_t type_to = 0;

	Inst_Cast (uint8_t reg, uint8_t type_from, uint8_t type_to) {
		this->bytecode = BYTECODE_CAST;
		this->reg = reg;
		this->type_from = type_from;
		this->type_to = type_to;
	}
};

struct Inst_Set : Instruction {
	uint8_t reg = 0;
	uint8_t bytecode_type = 0;
	uint8_t* data = NULL;

	Inst_Set (uint8_t reg, uint8_t value)  : Inst_Set (reg, BYTECODE_TYPE_U8,  &value) {}
	Inst_Set (uint8_t reg, uint16_t value) : Inst_Set (reg, BYTECODE_TYPE_U16, &value) {}
	Inst_Set (uint8_t reg, uint32_t value) : Inst_Set (reg, BYTECODE_TYPE_U32, &value) {}
	Inst_Set (uint8_t reg, uint64_t value) : Inst_Set (reg, BYTECODE_TYPE_U64, &value) {}
	Inst_Set (uint8_t reg, int8_t value)   : Inst_Set (reg, BYTECODE_TYPE_S8,  &value) {}
	Inst_Set (uint8_t reg, int16_t value)  : Inst_Set (reg, BYTECODE_TYPE_S16, &value) {}
	Inst_Set (uint8_t reg, int32_t value)  : Inst_Set (reg, BYTECODE_TYPE_S32, &value) {}
	Inst_Set (uint8_t reg, int64_t value)  : Inst_Set (reg, BYTECODE_TYPE_S64, &value) {}
	Inst_Set (uint8_t reg, float value)    : Inst_Set (reg, BYTECODE_TYPE_F32, &value) {}
	Inst_Set (uint8_t reg, double value)   : Inst_Set (reg, BYTECODE_TYPE_F64, &value) {}
	Inst_Set (uint8_t reg, uint8_t bytecode_type, void* data) {
		this->bytecode = BYTECODE_SET;
		this->reg = reg;
		this->bytecode_type = bytecode_type;
		auto size = bytecode_get_size(bytecode_type);
		this->data = (uint8_t*) malloc(size);
		memcpy(this->data, data, size);
	}
};

struct Inst_Constant_Offset : Instruction {
	uint8_t reg = 0;
	uint8_t offset = 0;

	Inst_Constant_Offset (uint8_t reg, uint8_t offset) {
		this->bytecode = BYTECODE_CONSTANT_OFFSET;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Global_Offset : Instruction {
	uint8_t reg = 0;
	uint8_t offset = 0;

	Inst_Global_Offset (uint8_t reg, uint8_t offset) {
		this->bytecode = BYTECODE_GLOBAL_OFFSET;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Stack_Allocate : Instruction {
	uint8_t size = 0;

	Inst_Stack_Allocate (uint8_t size) {
		this->bytecode = BYTECODE_STACK_ALLOCATE;
		this->size = size;
	}
};

struct Inst_Stack_Free : Instruction {
	uint8_t size = 0;

	Inst_Stack_Free (uint8_t size) {
		this->bytecode = BYTECODE_STACK_FREE;
		this->size = size;
	}
};

struct Inst_Stack_Offset : Instruction {
	uint8_t reg = 0;
	uint8_t offset = 0;

	Inst_Stack_Offset (uint8_t reg, uint8_t offset) {
		this->bytecode = BYTECODE_STACK_OFFSET;
		this->reg = reg;
		this->offset = offset;
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

struct Inst_Unary : Instruction {
	uint8_t unop = 0;
	uint8_t reg = 0;
	uint8_t bytecode_type = 0;

	Inst_Unary (uint8_t unop, uint8_t reg, uint8_t bytecode_type) {
		this->bytecode = BYTECODE_UNARY;
		this->unop = unop;
		this->reg = reg;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Binary : Instruction {
	uint8_t binop = 0;
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;
	uint8_t bytecode_type = 0;

	Inst_Binary (uint8_t binop, uint8_t reg1, uint8_t reg2, uint8_t bytecode_type) {
		this->bytecode = BYTECODE_BINARY;
		this->binop = binop;
		this->reg1 = reg1;
		this->reg2 = reg2;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Jump : Instruction {
	int32_t offset;

	Inst_Jump (int32_t offset = 0) {
		this->bytecode = BYTECODE_JUMP;
		this->offset = offset;
	}
};

struct Inst_Jump_If_False : Instruction {
	uint8_t reg;
	int32_t offset;

	Inst_Jump_If_False (uint8_t reg, int32_t offset = 0) {
		this->bytecode = BYTECODE_JUMP_IF_FALSE;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Call_Setup : Instruction {
	uint8_t calling_convention;

	Inst_Call_Setup (uint8_t calling_convention) {
		this->bytecode = BYTECODE_CALL_SETUP;
		this->calling_convention = calling_convention;
	}
};

struct Inst_Call_Param : Instruction {
	uint8_t index;
	uint8_t bytecode_type;

	Inst_Call_Param (uint8_t index, uint8_t bytecode_type) {
		this->bytecode = BYTECODE_CALL_PARAM;
		this->index = index;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Call : Instruction {
	uint8_t reg;
	size_t function_pointer;

	Inst_Call (size_t function_pointer) {
		this->bytecode = BYTECODE_CALL;
		this->function_pointer = function_pointer;
	}
};

struct Inst_Return : Instruction {
	Inst_Return () { this->bytecode = BYTECODE_RETURN; }
};
