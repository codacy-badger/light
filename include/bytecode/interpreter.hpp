#pragma once

#include "bytecode/instructions.hpp"
#include "bytecode/foreign_functions.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"
#include "bytecode/types.hpp"

#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

#include <deque>

#include "dyncall/dyncall.h"

#define INTERP_REGISTER_SIZE  sizeof(size_t)
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  64

#define INTERP_STACK_REGISTER INTERP_REGISTER_COUNT - 1

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Bytecode_Interpreter {
	Bytecode_Generator* generator = new Bytecode_Generator();
	Bytecode_Runner* runner = new Bytecode_Runner();

	Foreign_Functions* foreign_functions = new Foreign_Functions();
	Bytecode_Constants* constants = new Bytecode_Constants();
	Bytecode_Globals* globals = new Bytecode_Globals();
	Bytecode_Types* types = new Bytecode_Types();
	bool stop_running = false;

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	uint8_t stack[INTERP_STACK_SIZE];
	size_t instruction_index = 0;
	size_t stack_index = 0;
	size_t stack_base = 0;

	DCCallVM* vm = NULL;

	Bytecode_Interpreter (size_t vm_size = 64);
	~Bytecode_Interpreter ();

	void set (Ast_Comma_Separated_Arguments* args);
	void run (Ast_Function* func);
	void run (Instruction* inst);
	void print (size_t index, Instruction* inst);

	void dump ();
};
