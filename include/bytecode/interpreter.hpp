#pragma once

#include "platform.hpp"
#include "bytecode/instructions.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"
#include "bytecode/call_record.hpp"

#include <assert.h>

#include "dyncall/dyncall.h"

#define INTERP_REGISTER_SIZE  8
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  (1024 * 1024 * 1024) // 1 MB

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Interpreter {
	Call_Record<Bytecode_Register>* call_record = new Call_Record<Bytecode_Register>();
	Bytecode_Constants* constants = new Bytecode_Constants();
	Bytecode_Globals* globals = new Bytecode_Globals();

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	uint8_t stack[INTERP_STACK_SIZE];
	size_t instruction_index = 0;
	size_t stack_index = 0;
	size_t stack_base = 0;

	DCCallVM* vm = NULL;

	Interpreter (size_t vm_size = 512) {
		assert(INTERP_REGISTER_SIZE >= sizeof(void*));
		memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
		this->vm = dcNewCallVM(vm_size);
	}

	~Interpreter () { dcFree(this->vm); }

	void run (Ast_Function* func);
	void run (Instruction* inst);

	void call (void* func_ptr, Bytecode_Type bytecode_type, uint8_t reg_result);
};
