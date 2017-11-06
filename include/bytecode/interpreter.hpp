#pragma once

#include "bytecode/instructions.hpp"

#include <deque>

#define INTERP_REGISTER_SIZE  8
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  256

#define INTERP_STACK_REGISTER INTERP_REGISTER_COUNT - 1

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Bytecode_Interpreter {
	bool stop_running = false;

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	std::deque<uint8_t*> stack_ptrs;

	bool flag_carry = false;

	Bytecode_Interpreter ();

	void run (Instruction* inst);

	void copy_const (Inst_Copy_Const* inst);
	void copy_reg (Inst_Copy_Reg* inst);

	void stack_alloca (Inst_Stack_Alloca* inst);

	void add_i32 (Inst_Add_I32* inst);
	void add_reg (Inst_Add_Reg* inst);

	void dump ();
};
