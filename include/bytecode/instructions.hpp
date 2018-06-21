#pragma once

#include <stdint.h>
#include <malloc.h>

#include "parser/ast.hpp"
#include "report.hpp"

struct Interpreter;

enum Inst_Bytecode : uint8_t {
	BYTECODE_NOOP = 0,

	BYTECODE_COPY,
	BYTECODE_COPY_MEMORY,
	BYTECODE_CAST,

	BYTECODE_SET,

	BYTECODE_CONSTANT_OFFSET,
	BYTECODE_GLOBAL_OFFSET,
	BYTECODE_STACK_ALLOCATE,
	BYTECODE_STACK_OFFSET,

	BYTECODE_LOAD,
	BYTECODE_STORE,

	BYTECODE_UNARY,
	BYTECODE_BINARY,

	BYTECODE_ADD_CONST,
	BYTECODE_MUL_CONST,

	BYTECODE_JUMP,
	BYTECODE_JUMP_IF_FALSE,

	BYTECODE_CALL_SETUP,
	BYTECODE_CALL_PARAM,
	BYTECODE_CALL,
	BYTECODE_CALL_CONST,

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
	BYTECODE_REM,

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

enum Bytecode_Type : uint8_t {
	BYTECODE_TYPE_UNINITIALIZED = 0,
	BYTECODE_TYPE_VOID,
	BYTECODE_TYPE_BOOL,
	BYTECODE_TYPE_S8,
	BYTECODE_TYPE_S16,
	BYTECODE_TYPE_S32,
	BYTECODE_TYPE_S64,
	BYTECODE_TYPE_U8,
	BYTECODE_TYPE_U16,
	BYTECODE_TYPE_U32,
	BYTECODE_TYPE_U64,
	BYTECODE_TYPE_F32,
	BYTECODE_TYPE_F64,
	BYTECODE_TYPE_POINTER,
};

Bytecode_Type bytecode_get_type (Ast_Type_Instance* decl_ty);
Bytecode_Type bytecode_get_type (Ast_Expression* exp);
size_t bytecode_get_size (Bytecode_Type bytecode_type);
bool bytecode_has_sign (Bytecode_Type bytecode_type);
Bytecode_Type bytecode_unsigned_to_signed (Bytecode_Type bytecode_type);

struct Instruction {
	Inst_Bytecode bytecode = BYTECODE_NOOP;
	Location location;
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

struct Inst_Copy_Memory : Instruction {
	uint8_t reg_to = 0;
	uint8_t reg_from = 0;
	size_t size = 0;

	Inst_Copy_Memory (uint8_t reg_to, uint8_t reg_from, size_t size) {
		this->bytecode = BYTECODE_COPY_MEMORY;
		this->reg_to = reg_to;
		this->reg_from = reg_from;
		this->size = size;
	}
};

struct Inst_Cast : Instruction {
	uint8_t reg_to = 0;
	uint8_t reg_from = 0;
	Bytecode_Type type_from = BYTECODE_TYPE_UNINITIALIZED;
	Bytecode_Type type_to = BYTECODE_TYPE_UNINITIALIZED;

	Inst_Cast (uint8_t reg_to, uint8_t reg_from, Bytecode_Type type_from, Bytecode_Type type_to) {
		this->bytecode = BYTECODE_CAST;
		this->reg_to = reg_to;
		this->reg_from = reg_from;
		this->type_from = type_from;
		this->type_to = type_to;
	}

	Inst_Cast (uint8_t reg, Bytecode_Type type_from, Bytecode_Type type_to) {
		this->bytecode = BYTECODE_CAST;
		this->reg_to = reg;
		this->reg_from = reg;
		this->type_from = type_from;
		this->type_to = type_to;
	}
};

struct Inst_Set : Instruction {
	uint8_t reg = 0;
	Bytecode_Type bytecode_type = BYTECODE_TYPE_UNINITIALIZED;
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
	Inst_Set (uint8_t reg, Bytecode_Type bytecode_type, void* data) {
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
	size_t offset = 0;

	Inst_Constant_Offset (uint8_t reg, size_t offset) {
		this->bytecode = BYTECODE_CONSTANT_OFFSET;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Global_Offset : Instruction {
	uint8_t reg = 0;
	size_t offset = 0;

	Inst_Global_Offset (uint8_t reg, size_t offset) {
		this->bytecode = BYTECODE_GLOBAL_OFFSET;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Stack_Allocate : Instruction {
	size_t size = 0;

	Inst_Stack_Allocate (size_t size) {
		this->bytecode = BYTECODE_STACK_ALLOCATE;
		this->size = size;
	}
};

struct Inst_Stack_Offset : Instruction {
	uint8_t reg = 0;
	size_t offset = 0;

	Inst_Stack_Offset (uint8_t reg, size_t offset) {
		this->bytecode = BYTECODE_STACK_OFFSET;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Load : Instruction {
	uint8_t dest = 0;
	uint8_t src = 0;
	size_t size = 0;

	Inst_Load (uint8_t dest, uint8_t src, size_t size) {
		this->bytecode = BYTECODE_LOAD;
		this->dest = dest;
		this->src = src;
		this->size = size;
	}
};

struct Inst_Store : Instruction {
	uint8_t dest = 0;
	uint8_t src = 0;
	size_t size = 0;

	Inst_Store (uint8_t dest, uint8_t src, size_t size) {
		this->bytecode = BYTECODE_STORE;
		this->dest = dest;
		this->src = src;
		this->size = size;
	}
};

struct Inst_Unary : Instruction {
	uint8_t unop = 0;
	uint8_t target = 0;
	uint8_t reg = 0;
	Bytecode_Type bytecode_type = BYTECODE_TYPE_UNINITIALIZED;

	Inst_Unary (uint8_t unop, uint8_t reg, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_UNARY;
		this->unop = unop;
		this->target = reg;
		this->reg = reg;
		this->bytecode_type = bytecode_type;
	}

	Inst_Unary (uint8_t unop, uint8_t target, uint8_t reg, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_UNARY;
		this->unop = unop;
		this->target = target;
		this->reg = reg;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Binary : Instruction {
	uint8_t binop = 0;
	uint8_t target = 0;
	uint8_t reg1 = 0;
	uint8_t reg2 = 0;
	Bytecode_Type bytecode_type = BYTECODE_TYPE_UNINITIALIZED;

	Inst_Binary (uint8_t binop, uint8_t reg1, uint8_t reg2, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_BINARY;
		this->binop = binop;
		this->target = reg1;
		this->reg1 = reg1;
		this->reg2 = reg2;
		this->bytecode_type = bytecode_type;
	}

	Inst_Binary (uint8_t binop, uint8_t target, uint8_t reg1, uint8_t reg2, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_BINARY;
		this->binop = binop;
		this->target = target;
		this->reg1 = reg1;
		this->reg2 = reg2;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Add_Const : Instruction {
	uint8_t target = 0;
	uint8_t reg = 0;
	uint64_t number = 0;

	Inst_Add_Const (uint8_t reg, uint64_t number) {
		this->bytecode = BYTECODE_ADD_CONST;
		this->target = reg;
		this->reg = reg;
		this->number = number;
	}

	Inst_Add_Const (uint8_t target, uint8_t reg, uint64_t number) {
		this->bytecode = BYTECODE_ADD_CONST;
		this->target = target;
		this->reg = reg;
		this->number = number;
	}
};

struct Inst_Mul_Const : Instruction {
	uint8_t target = 0;
	uint8_t reg = 0;
	uint64_t number = 0;

	Inst_Mul_Const (uint8_t reg, uint64_t number) {
		this->bytecode = BYTECODE_MUL_CONST;
		this->target = reg;
		this->reg = reg;
		this->number = number;
	}

	Inst_Mul_Const (uint8_t target, uint8_t reg, uint64_t number) {
		this->bytecode = BYTECODE_MUL_CONST;
		this->target = target;
		this->reg = reg;
		this->number = number;
	}
};

struct Inst_Jump : Instruction {
	size_t offset;

	Inst_Jump (size_t offset = 0) {
		this->bytecode = BYTECODE_JUMP;
		this->offset = offset;
	}
};

struct Inst_Jump_If_False : Instruction {
	uint8_t reg;
	size_t offset;

	Inst_Jump_If_False (uint8_t reg, size_t offset = 0) {
		this->bytecode = BYTECODE_JUMP_IF_FALSE;
		this->reg = reg;
		this->offset = offset;
	}
};

struct Inst_Call_Setup : Instruction {
	uint8_t calling_convention;
	uint8_t param_count;

	Inst_Call_Setup (uint8_t calling_convention, uint8_t param_count) {
		this->bytecode = BYTECODE_CALL_SETUP;
		this->calling_convention = calling_convention;
		this->param_count = param_count;
	}
};

struct Inst_Call_Param : Instruction {
	uint8_t reg_index;
	uint8_t param_index;
	Bytecode_Type bytecode_type;

	Inst_Call_Param (uint8_t param_index, uint8_t reg_index, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_CALL_PARAM;
		this->reg_index = reg_index;
		this->param_index = param_index;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Call : Instruction {
	uint8_t reg_function;
	uint8_t reg_result;
	Bytecode_Type bytecode_type;

	Inst_Call (uint8_t reg_result, uint8_t reg_function, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_CALL;
		this->reg_result = reg_result;
		this->reg_function = reg_function;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Call_Const : Instruction {
	uint64_t address;
	uint8_t reg_result;
	Bytecode_Type bytecode_type;

	Inst_Call_Const (uint64_t address, uint8_t reg_result, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_CALL_CONST;
		this->address = address;
		this->reg_result = reg_result;
		this->bytecode_type = bytecode_type;
	}
};

struct Inst_Return : Instruction {
	uint8_t reg_index;
	Bytecode_Type bytecode_type;

	Inst_Return (uint8_t reg_index, Bytecode_Type bytecode_type) {
		this->bytecode = BYTECODE_RETURN;
		this->reg_index = reg_index;
		this->bytecode_type = bytecode_type;
	}

	Inst_Return () {
		this->bytecode = BYTECODE_RETURN;
		this->bytecode_type = BYTECODE_TYPE_VOID;
		this->reg_index = 0;
	}
};
