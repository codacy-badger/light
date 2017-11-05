#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INST_OFFSET(offset) buffer[offset]

Bytecode_Interpreter::Bytecode_Interpreter () {
	assert(REGISTER_SIZE >= sizeof(void*));
	// INFO: not necessary, but makes debug easier
	memset(&this->registers, 0, REGISTER_COUNT * sizeof(Bytecode_Register));
}

void Bytecode_Interpreter::run (uint8_t* buffer) {
	uint8_t incr = 0;
	while (!this->stop_running) {
		buffer += this->run_next(buffer);
	}
	printf("Bytecode interpreter completed!\n");
}

uint8_t Bytecode_Interpreter::run_next (uint8_t* buffer) {
	uint8_t op = *(buffer++);
	switch (op) {
		case BYTECODE_NOOP: return 0;
		case BYTECODE_STOP: {
			this->stop_running = true;
			return 0;
		}
		case BYTECODE_SET: {
			uint8_t size = INST_OFFSET(0);
			this->set(size, INST_OFFSET(1), &INST_OFFSET(2));
			return 3 + size;
		}
		case BYTECODE_COPY: {
			this->copy(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			return 4;
		}
		case BYTECODE_STACK_ALLOCA: {
			uint8_t size = INST_OFFSET(0);
			this->stack_alloca(size, &INST_OFFSET(1));
			return 2 + size;
		}
		case BYTECODE_STACK_OFFSET: {
			uint8_t size = INST_OFFSET(0);
			this->stack_offset(size, INST_OFFSET(1), &INST_OFFSET(2));
			return 3 + size;
		}
		case BYTECODE_STORE_INT: {
			uint8_t size = INST_OFFSET(0);
			this->store_int(size, INST_OFFSET(1), &INST_OFFSET(2));
			return 3 + size;
		}
		case BYTECODE_STORE: {
			this->store(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			return 4;
		}
		case BYTECODE_LOAD: {
			this->load(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			return 4;
		}
		case BYTECODE_ADD_INT: {
			uint8_t size = INST_OFFSET(0);
			this->add_int(size, INST_OFFSET(1), &INST_OFFSET(2));
			return 3 + size;
		}
		case BYTECODE_ADD: {
			this->add(INST_OFFSET(0), INST_OFFSET(1), INST_OFFSET(2));
			return 4;
		}
		default: {
			printf("--- UNKNOWN BYTECODE OP ---\n");
			printf("OP: %d (%02X)\n", (*buffer), (*buffer));
			this->stop_running = true;
			return 0;
		}
	}
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

void Bytecode_Interpreter::store (uint8_t size, uint8_t reg1, uint8_t reg2) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
	memcpy(ptr, this->registers[reg2], size);
}

void Bytecode_Interpreter::store_int (uint8_t size, uint8_t reg1, uint8_t* data) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, this->registers[reg1], sizeof(uint8_t*));
	memcpy(ptr, data, size);
}

void Bytecode_Interpreter::load (uint8_t size, uint8_t reg1, uint8_t reg2) {
	uint8_t* ptr = NULL;
	memcpy(&ptr, &this->registers[reg2], sizeof(uint8_t*));
	memcpy(this->registers[reg1], ptr, size);
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

void Bytecode_Interpreter::dump () {
	printf("\n------------ Light VM dump ------------\n\n");
	for (short i = 0; i < REGISTER_COUNT; i++) {
        for (size_t j = REGISTER_SIZE; j > 0; j--) {
            printf("%02X", this->registers[i][j - 1]);
		}
		if ((i + 1) % 4 == 0) printf("\n");
		else printf("  ");
	}
	printf("\nStack [%lld / %d]\n\n\t", this->stack_index, STACK_SIZE);
	for (size_t i = 0; i < this->stack_index; i++) {
		printf("%02X ", this->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n\n");
}
