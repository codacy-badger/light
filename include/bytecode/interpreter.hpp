#pragma once

#include "bytecode/instructions.hpp"
#include "bytecode/foreign_functions.hpp"
#include "bytecode/globals.hpp"

#include <deque>

#include "dyncall/dyncall.h"

#define INTERP_REGISTER_SIZE  sizeof(size_t)
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  255

#define INTERP_STACK_REGISTER INTERP_REGISTER_COUNT - 1

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Bytecode_Interpreter {
	Foreign_Functions* foreign_functions = new Foreign_Functions();
	Bytecode_Globals* globals = new Bytecode_Globals();
	bool stop_running = false;

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	uint8_t stack[INTERP_STACK_SIZE];
	size_t stack_index = 0;
	size_t stack_base = 0;

	DCCallVM* vm = NULL;

	Bytecode_Interpreter (size_t vm_size = 64);
	~Bytecode_Interpreter ();

	void run (Ast_Function* func);
	void run (Instruction* inst);
	void print (size_t index, Instruction* inst);

	void dump ();
};
