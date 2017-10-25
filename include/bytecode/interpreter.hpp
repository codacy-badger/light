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

	bool run_next ();

	void set (uint8_t size, uint8_t reg1, uint8_t* data);
	void copy (uint8_t size, uint8_t reg1, uint8_t reg2);
	void stack_alloca (uint8_t size, uint8_t* data);
	void stack_offset (uint8_t size, uint8_t reg1, uint8_t* data);
	void store_int (uint8_t size, uint8_t reg1, uint8_t* data);
	void store (uint8_t size, uint8_t reg1, uint8_t reg2);
	void load (uint8_t size, uint8_t reg1, uint8_t reg2);
	void add_int (uint8_t size, uint8_t reg1, uint8_t* data);
	void add (uint8_t size, uint8_t reg1, uint8_t reg2);

	void dump ();
};
