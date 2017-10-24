#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

Bytecode_Interpreter::Bytecode_Interpreter () {
	assert(REGISTER_SIZE >= sizeof(uint8_t*));
	// INFO: this makes debug easier
	memset(&this->registers, 0, REGISTER_COUNT * sizeof(Bytecode_Register));
}

void Bytecode_Interpreter::start () {
	while (this->instructions[this->instruction_index] != BYTECODE_STOP) {
		this->run_next();
	}
	printf("Execution stop!\n");
}

void Bytecode_Interpreter::run_next () {
	switch (this->instructions[this->instruction_index]) {
		case BYTECODE_NOOP: {
			this->instruction_index += 1;
			break;
		}
		case BYTECODE_SET: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg = this->instructions[this->instruction_index + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_index + 3];

			memcpy(&this->registers[reg], dataPtr, size);

			this->instruction_index += 3 + size;
			break;
		}
		case BYTECODE_COPY: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t reg2 = this->instructions[this->instruction_index + 3];

			memcpy(this->registers[reg1], this->registers[reg2], size);

			this->instruction_index += 4;
			break;
		}
		case BYTECODE_STACK_ALLOCA: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t* dataPtr = &this->instructions[this->instruction_index + 2];

			uint64_t _tmp = 0;
			for (size_t i = 0; i < size; i++) {
				_tmp = *(dataPtr + i) << (8 * (size - i - 1));
				this->stack_index += _tmp;
			}

			this->instruction_index += 2 + size;
			break;
		}
		case BYTECODE_STACK_OFFSET: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t* offsetPtr = &this->instructions[this->instruction_index + 3];

			uint8_t** reg_ptr = reinterpret_cast<uint8_t**>(this->registers[reg1]);
			uint8_t* current_stack_ptr = &this->stack[this->stack_index];
			*reg_ptr = current_stack_ptr;

			uint64_t _tmp = 0;
			for (size_t i = 0; i < size; i++) {
				_tmp = *(offsetPtr + i) << (8 * (size - i - 1));
				*reg_ptr -= _tmp;
			}

			this->instruction_index += 3 + size;
			break;
		}
		case BYTECODE_STORE_I: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_index + 3];

			uint8_t* ptr = NULL;
			memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
			memcpy(ptr, dataPtr, size);

			this->instruction_index += 3 + size;
			break;
		}
		case BYTECODE_STORE: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t reg2 = this->instructions[this->instruction_index + 3];

			uint8_t* ptr = NULL;
			memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
			memcpy(ptr, this->registers[reg2], size);

			this->instruction_index += 4;
			break;
		}
		case BYTECODE_LOAD: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t reg2 = this->instructions[this->instruction_index + 3];

			uint8_t* ptr = NULL;
			memcpy(&ptr, &this->registers[reg2], sizeof(uint8_t*));
			memcpy(this->registers[reg1], ptr, size);

			this->instruction_index += 4;
			break;
		}
		case BYTECODE_ADD_I: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t* dataPtr = &this->instructions[this->instruction_index + 3];

			uint16_t _tmp = 0;
		    for (size_t i = 0; i < size; i++) {
		        _tmp = this->registers[reg1][i] + *(dataPtr + i);
		        if (this->flag_carry) _tmp += 1;
		        this->flag_carry = (_tmp >> 8) > 0;
		        this->registers[reg1][i] = _tmp;
		    }
		    if (this->flag_carry) printf("Carry!\n");

			this->instruction_index += 3 + size;
			break;
		}
		case BYTECODE_ADD: {
			uint8_t size = this->instructions[this->instruction_index + 1];
			uint8_t reg1 = this->instructions[this->instruction_index + 2];
			uint8_t reg2 = this->instructions[this->instruction_index + 3];

			uint16_t _tmp = 0;
		    for (size_t i = 0; i < size; i++) {
		        _tmp = this->registers[reg1][i] + this->registers[reg2][i];
		        if (this->flag_carry) _tmp += 1;
		        this->flag_carry = (_tmp >> 8) > 0;
		        this->registers[reg1][i] = _tmp;
		    }

			this->instruction_index += 4;
			break;
		}
		default: {
			printf("--- UNKNOWN BYTECODE OP ---\n");
			printf("OP: %d\nAt: %lld", this->instructions[this->instruction_index], this->instruction_index);
			exit(1);
		}
	}
}

void Bytecode_Interpreter::dump () {
	printf("\n------------ Light VM DUMP ------------\n\n");
	printf("Instruction: %016llx\n\n", this->instruction_index);
	for (short i = 0; i < REGISTER_COUNT; i++) {
        for (size_t j = REGISTER_SIZE; j > 0; j--) {
            printf("%02X", this->registers[i][j - 1]);
		}
		if ((i + 1) % 4 == 0) printf("\n");
		else printf("  ");
	}
	printf("\nStack [%lld / %d]\n\n\t", this->stack_index, DEFAULT_STACK_SIZE);
	for (size_t i = 0; i < this->stack_index; i++) {
		printf("%02X ", this->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n");
}
