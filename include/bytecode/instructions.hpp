#pragma once

#include <stdint.h>
#include <malloc.h>
#include <string.h>

#include "parser/ast.hpp"

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

	BYTECODE_CALL_SETUP,
	BYTECODE_CALL_PARAM,
	BYTECODE_CALL_FOREIGN,
	BYTECODE_CALL,
	BYTECODE_RETURN,
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

const uint8_t BYTECODE_CC_DEFAULT   = 0x0;
const uint8_t BYTECODE_CC_CDECL     = 0x1;
const uint8_t BYTECODE_CC_STDCALL   = 0x2;
const uint8_t BYTECODE_CC_FASTCALL  = 0x3;

const uint8_t BYTECODE_TYPE_VOID 	= 0x0;
const uint8_t BYTECODE_TYPE_S8 		= 0x1;
const uint8_t BYTECODE_TYPE_S16 	= 0x2;
const uint8_t BYTECODE_TYPE_S32 	= 0x3;
const uint8_t BYTECODE_TYPE_S64 	= 0x4;
const uint8_t BYTECODE_TYPE_U8 		= 0x5;
const uint8_t BYTECODE_TYPE_U16 	= 0x6;
const uint8_t BYTECODE_TYPE_U32 	= 0x7;
const uint8_t BYTECODE_TYPE_U64 	= 0x8;
const uint8_t BYTECODE_TYPE_F32 	= 0x9;
const uint8_t BYTECODE_TYPE_F64 	= 0xA;
const uint8_t BYTECODE_TYPE_POINTER	= 0xB;
const uint8_t BYTECODE_TYPE_STRING 	= 0xC;

uint8_t bytecode_get_type (Ast_Type_Definition* decl_ty);
uint8_t bytecode_get_type (Ast_Expression* exp);

struct Inst_Call_Setup : Instruction {
	uint8_t calling_convention = BYTECODE_CC_DEFAULT;
	uint8_t is_native = 0;

	Inst_Call_Setup (uint8_t calling_convention, bool is_native) {
		this->bytecode = BYTECODE_CALL_SETUP;
		this->calling_convention = calling_convention;
		this->is_native = is_native;
	}
};

struct Inst_Call_Param : Instruction {
	uint8_t index = 0;
	uint8_t reg = 0;
	uint8_t bytecode_type = 0;

	Inst_Call_Param (uint8_t index, uint8_t reg, uint8_t bytecode_type) {
		this->bytecode = BYTECODE_CALL_PARAM;
		this->index = index;
		this->reg = reg;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Call_Foreign : Instruction {
	uint8_t reg = 0;
	uint8_t module_index = 0;
	uint8_t function_index = 0;
	uint8_t bytecode_type = 0;

	Inst_Call_Foreign (uint8_t reg, size_t module_index, size_t function_index, uint8_t bytecode_type) {
		this->bytecode = BYTECODE_CALL_FOREIGN;
		this->reg = reg;
		this->module_index = module_index;
		this->function_index = function_index;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Call : Instruction {
	uint8_t reg = 0;
	size_t function_pointer = 0;

	Inst_Call (uint8_t reg, size_t function_pointer) {
		this->bytecode = BYTECODE_CALL;
		this->reg = reg;
		this->function_pointer = function_pointer;
	}
};

struct Inst_Return : Instruction {
	Inst_Return () { this->bytecode = BYTECODE_RETURN; }
};
