#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INST_OFFSET(offset) this->instructions[this->instruction_index + (offset)]

Bytecode_Interpreter::Bytecode_Interpreter () {
	assert(REGISTER_SIZE >= sizeof(uint8_t*));
	// INFO: this makes debug easier
	memset(&this->registers, 0, REGISTER_COUNT * sizeof(Bytecode_Register));
}

void Bytecode_Interpreter::start () {
	bool ok = true;
	while (INST_OFFSET(0) != BYTECODE_STOP) {
		ok = this->run_next();
		if (!ok) break;
	}
	if (!ok) {
		printf("ERROR!\n");
		this->dump();
	} else printf("Terminated successfully!\n");
}

bool Bytecode_Interpreter::run_next () {
	uint8_t op = INST_OFFSET(0);
	this->instruction_index++;
	switch (op) {
		case BYTECODE_NOOP: break;
		case BYTECODE_SET: {
			uint8_t size = INST_OFFSET(0);
			this->set(size, INST_OFFSET(1), &INST_OFFSET(2));
			this->instruction_index += 2 + size;
			break;
		}
		case BYTECODE_COPY: {
			this->copy(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			this->instruction_index += 3;
			break;
		}
		case BYTECODE_STACK_ALLOCA: {
			uint8_t size = INST_OFFSET(0);
			this->stack_alloca(size, &INST_OFFSET(1));
			this->instruction_index += 1 + size;
			break;
		}
		case BYTECODE_STACK_OFFSET: {
			uint8_t size = INST_OFFSET(0);
			this->stack_offset(size, INST_OFFSET(1), &INST_OFFSET(2));
			this->instruction_index += 2 + size;
			break;
		}
		case BYTECODE_STORE_INT: {
			uint8_t size = INST_OFFSET(0);
			this->store_int(size, INST_OFFSET(1), &INST_OFFSET(2));
			this->instruction_index += 2 + size;
			break;
		}
		case BYTECODE_STORE: {
			this->store(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			this->instruction_index += 3;
			break;
		}
		case BYTECODE_LOAD: {
			this->load(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			this->instruction_index += 3;
			break;
		}
		case BYTECODE_ADD_INT: {
			uint8_t size = INST_OFFSET(0);
			this->add_int(size, INST_OFFSET(1), &INST_OFFSET(2));
			this->instruction_index += 2 + size;
			break;
		}
		case BYTECODE_ADD: {
			this->add(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			this->instruction_index += 3;
			break;
		}
		default: {
			printf("--- UNKNOWN BYTECODE OP ---\n");
			printf("OP: %d\nAt: %lld", this->instructions[this->instruction_index], this->instruction_index);
			return false;
		}
	}
	return true;
}

void Bytecode_Interpreter::set (uint8_t size, uint8_t reg1, uint8_t* data) {
	memcpy(&this->registers[reg1], data, size);
}

void Bytecode_Interpreter::copy (uint8_t size, uint8_t reg1, uint8_t reg2) {
	memcpy(this->registers[reg1], this->registers[reg2], size);
}

void Bytecode_Interpreter::stack_alloca (uint8_t size, uint8_t* data) {
	uint64_t _tmp = 0;
	for (size_t i = 0; i < size; i++) {
		_tmp = *(data + i) << (8 * (size - i - 1));
		this->stack_index += _tmp;
	}
}

void Bytecode_Interpreter::stack_offset (uint8_t size, uint8_t reg1, uint8_t* data) {
	uint8_t** reg_ptr = reinterpret_cast<uint8_t**>(this->registers[reg1]);
	uint8_t* current_stack_ptr = &this->stack[this->stack_index];
	*reg_ptr = current_stack_ptr;

	uint64_t _tmp = 0;
	for (size_t i = 0; i < size; i++) {
		_tmp = *(data + i) << (8 * (size - i - 1));
		*reg_ptr -= _tmp;
	}
}

void Bytecode_Interpreter::store_int (uint8_t size, uint8_t reg1, uint8_t* data) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
	memcpy(ptr, data, size);
}

void Bytecode_Interpreter::store (uint8_t size, uint8_t reg1, uint8_t reg2) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
	memcpy(ptr, this->registers[reg2], size);
}

void Bytecode_Interpreter::load (uint8_t size, uint8_t reg1, uint8_t reg2) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, &this->registers[reg2], sizeof(uint8_t*));
	memcpy(this->registers[reg1], ptr, size);
}

void Bytecode_Interpreter::add_int (uint8_t size, uint8_t reg1, uint8_t* data) {
	uint16_t _tmp = 0;
	for (size_t i = 0; i < size; i++) {
		_tmp = this->registers[reg1][i] + *(data + i);
		if (this->flag_carry) _tmp += 1;
		this->flag_carry = (_tmp >> 8) > 0;
		this->registers[reg1][i] = _tmp;
	}
	if (this->flag_carry) printf("Carry!\n");
}

void Bytecode_Interpreter::add (uint8_t size, uint8_t reg1, uint8_t reg2) {
	uint16_t _tmp = 0;
	for (size_t i = 0; i < size; i++) {
		_tmp = this->registers[reg1][i] + this->registers[reg2][i];
		if (this->flag_carry) _tmp += 1;
		this->flag_carry = (_tmp >> 8) > 0;
		this->registers[reg1][i] = _tmp;
	}
	if (this->flag_carry) printf("Carry!\n");
}

void Bytecode_Interpreter::dump () {
	printf("\n------------ Light VM dump ------------\n\n");
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
