#pragma once

#include "bytecode/ops.hpp"

#define REGISTER_SIZE 		8
#define REGISTER_COUNT   	16
#define DEFAULT_STACK_SIZE 	256

typedef uint8_t Bytecode_Register[REGISTER_SIZE];

struct Bytecode_Interpreter {
	Bytecode_Register registers[REGISTER_COUNT];
	bool flag_carry = false;

	uint8_t* instructions = NULL;
	size_t instruction_index = 0;

	uint8_t stack[DEFAULT_STACK_SIZE];
	size_t stack_index = 0;

	Bytecode_Interpreter ();

	void start ();

	void run_next ();

	void dump ();
};
