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
			this->instruction_pointer += 1;
			break;
		}
		case BYTECODE_SET: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg = this->instructions[this->instruction_pointer + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_pointer + 3];

			memcpy(&this->registers[reg]._64, dataPtr, size);

			this->instruction_pointer += 3 + size;
			break;
		}
		case BYTECODE_COPY: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t reg2 = this->instructions[this->instruction_pointer + 3];

			memcpy(&this->registers[reg1]._64, &this->registers[reg2]._64, size);

			this->instruction_pointer += 4;
			break;
		}
		case BYTECODE_ALLOCA: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];

			uint8_t* current_stack_ptr = &this->stack[this->stack_pointer];
			this->registers[reg1]._64 = reinterpret_cast<uint64_t>(current_stack_ptr);

			this->stack_pointer += size;
			this->instruction_pointer += 3;
			break;
		}
		case BYTECODE_STORE_I: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_pointer + 3];

			memcpy(reinterpret_cast<uint8_t*>(this->registers[reg1]._64), dataPtr, size);

			this->instruction_pointer += 3 + size;
			break;
		}
		case BYTECODE_STORE: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t reg2 = this->instructions[this->instruction_pointer + 3];

			memcpy(reinterpret_cast<uint8_t*>(this->registers[reg1]._64),
				&this->registers[reg2]._64, size);

			this->instruction_pointer += 4;
			break;
		}
		case BYTECODE_LOAD: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t reg2 = this->instructions[this->instruction_pointer + 3];

			memcpy(&this->registers[reg1]._64, reinterpret_cast<uint8_t*>(this->registers[reg2]._64), size);

			this->instruction_pointer += 4;
			break;
		}
		case BYTECODE_ADD_I: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_pointer + 3];

			for (size_t i = 0; i < size; i++) {
				this->registers[reg1]._64 += (*dataPtr + i) << i;
			}

			this->instruction_pointer += 3 + size;
			break;
		}
		case BYTECODE_ADD: {
			uint8_t size = this->instructions[this->instruction_pointer + 1];
			uint8_t reg1 = this->instructions[this->instruction_pointer + 2];
			uint8_t reg2 = this->instructions[this->instruction_pointer + 3];

			uint8_t* dataPtr = &this->registers[reg2]._8;
			for (size_t i = 0; i < size; i++) {
				this->registers[reg1]._64 += (*dataPtr + i) << i;
			}

			this->instruction_pointer += 4;
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
	printf("\n------------ Light VM DUMP ------------\n\n");
	printf("Instruction PTR %016llx\n\n", this->instruction_pointer);
	for (short i = 0; i < NUM_OF_REGISTERS; i++) {
		printf("\t%016llx", this->registers[i]._64);
		if ((i + 1) % 4 == 0) printf("\n");
	}
	printf("\nStack size %016llx\n\n\t", this->stack_pointer);
	for (size_t i = 0; i < this->stack_pointer; i++) {
		printf("%02x ", this->stack[i]);
		if ((i + 1) % 8 == 0) printf("\n\t");
	}
	printf("\n");
}
