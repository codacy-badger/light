#pragma once

#include "bytecode/ops.hpp"

#define REGISTER_SIZE 	8
#define REGISTER_COUNT  16
#define STACK_SIZE 		256

typedef uint8_t Bytecode_Register[REGISTER_SIZE];

struct Bytecode_Interpreter {
	bool stop_running = false;

	Bytecode_Register registers[REGISTER_COUNT];
	bool flag_carry = false;

	uint8_t stack[STACK_SIZE];
	size_t stack_index = 0;

	Bytecode_Interpreter ();

	void run (uint8_t* buffer);
	uint8_t run_next (uint8_t* buffer);

	void set (uint8_t size, uint8_t reg1, uint8_t* data);
	void copy (uint8_t size, uint8_t reg1, uint8_t reg2);
	void stack_alloca (uint8_t size, uint8_t* data);
	void stack_offset (uint8_t size, uint8_t reg1, uint8_t* data);

	void store (uint8_t size, uint8_t reg1, uint8_t reg2);
	void store_int (uint8_t size, uint8_t reg1, uint8_t* data);
	void load (uint8_t size, uint8_t reg1, uint8_t reg2);

	void add (uint8_t size, uint8_t reg1, uint8_t reg2);
	void add_int (uint8_t size, uint8_t reg1, uint8_t* data);

	void dump ();
};
