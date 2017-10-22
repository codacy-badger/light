#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Bytecode_Interpreter::Bytecode_Interpreter () {
	memset(&this->registers, 0, NUM_OF_REGISTERS * sizeof(Bytecode_Register));
}

void Bytecode_Interpreter::start () {
	while (this->instructions[this->instruction_pointer] != BYTECODE_STOP) {
		this->run_next();
	}
	printf("Execution stop!\n");
}

void Bytecode_Interpreter::run_next () {
	switch (this->instructions[this->instruction_pointer]) {
		case BYTECODE_NOOP: {
			printf("BYTECODE_NOOP\n");
			this->instruction_pointer += 1;
			break;
		}
		case BYTECODE_ALLOCA: {
			printf("BYTECODE_ALLOCA\n");
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg = this->instructions[this->instruction_pointer + 2];
			this->registers[reg]._64 = reinterpret_cast<uint64_t>(&this->stack);
			this->stack_pointer += size;
			this->instruction_pointer += 3;
			break;
		}
		default: {
			printf("--- UNKNOWN BYTECODE OP ---\n");
			printf("OP: %d\nAt: %lld", this->instructions[this->instruction_pointer], this->instruction_pointer);
			exit(1);
		}
	}
}

void Bytecode_Interpreter::dump () {
	printf("Instruction PTR %016llx\n", this->instruction_pointer);
	printf("      Stack PTR %016llx\n", this->stack_pointer);
	for (short i = 0; i < NUM_OF_REGISTERS; i++)
		printf("         Reg%3d %016llx\n", i, this->registers[i]._64);
}
