#pragma once

#include "bytecode/instructions.hpp"
#include "bytecode/globals.hpp"

#include <deque>

#define INTERP_REGISTER_SIZE  sizeof(size_t)
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  256

#define INTERP_STACK_REGISTER INTERP_REGISTER_COUNT - 1

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Bytecode_Interpreter {
	Bytecode_Globals* globals = new Bytecode_Globals();
	bool stop_running = false;

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	uint8_t stack[INTERP_STACK_SIZE];
	size_t stack_index = 0;
	size_t stack_base = 0;

	bool flag_carry = false;

	Bytecode_Interpreter ();

	void run (Instruction* inst);

	void dump ();
};
