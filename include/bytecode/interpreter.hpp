#pragma once

#include "bytecode/ops.hpp"

struct Bytecode_Register {
	union {
		uint64_t _64;
		uint32_t _32;
		uint16_t _16;
		uint8_t _8;
	};
};

#define NUM_OF_REGISTERS 16
#define STACK_SIZE 100

struct Bytecode_Interpreter {
	Bytecode_Register registers[NUM_OF_REGISTERS];
	uint8_t stack[STACK_SIZE];

	uint8_t* instructions = NULL;
	uint64_t instruction_pointer = 0;
	uint64_t stack_pointer = 0;

	Bytecode_Interpreter ();

	void start ();

	void run_next ();

	void dump ();
};
