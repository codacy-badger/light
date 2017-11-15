#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.hpp"

#define INST_OFFSET(offset) buffer[offset]

Bytecode_Interpreter::Bytecode_Interpreter () {
	assert(INTERP_REGISTER_SIZE >= sizeof(void*));
	// INFO: not necessary, but makes debug easier
	memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
	uint8_t* stack = (uint8_t*) malloc(INTERP_STACK_SIZE);
	stack_ptrs.push_back(stack);
}

void Bytecode_Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
		case BYTECODE_NOOP: return;
		case BYTECODE_COPY_CONST: {
			this->copy_const(static_cast<Inst_Copy_Const*>(inst));
			return;
		}
		case BYTECODE_COPY_REG: {
			this->copy_reg(static_cast<Inst_Copy_Reg*>(inst));
			return;
		}
		case BYTECODE_STACK_ALLOCA: {
			this->stack_alloca(static_cast<Inst_Stack_Alloca*>(inst));
			return;
		}
		case BYTECODE_ADD_I32: {
			this->add_s32(static_cast<Inst_Add_I32*>(inst));
			return;
		}
		case BYTECODE_ADD_REG: {
			this->add_reg(static_cast<Inst_Add_Reg*>(inst));
			return;
		}
		default: {
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

uint64_t _tmp;

void Bytecode_Interpreter::copy_const (Inst_Copy_Const* inst) {
	assert(inst->size <= INTERP_REGISTER_SIZE);
	auto reg_ptr = static_cast<uint8_t*>(this->registers[inst->reg]);
	for (int i = 0; i < inst->size; i++)
		*(reg_ptr + i) = *(inst->data + i);
}

void Bytecode_Interpreter::copy_reg (Inst_Copy_Reg* inst) {
	memcpy(this->registers[inst->reg1], this->registers[inst->reg2], INTERP_REGISTER_SIZE);
}

void Bytecode_Interpreter::stack_alloca (Inst_Stack_Alloca* inst) {
	auto current_stack_ptr = this->stack_ptrs.back();
	memcpy(this->registers[inst->reg], &current_stack_ptr, INTERP_REGISTER_SIZE);
	this->stack_ptrs.push_back(this->stack_ptrs.back() + inst->size);
}

void Bytecode_Interpreter::add_s32 (Inst_Add_I32* inst) {
	// TODO: set overflow & underflow flags
	_tmp  = (*(inst->data + 0) << (8 * 0));
	_tmp += (*(inst->data + 1) << (8 * 1));
	_tmp += (*(inst->data + 2) << (8 * 2));
	_tmp += (*(inst->data + 3) << (8 * 3));

	auto reg_ptr = reinterpret_cast<uint64_t*>(this->registers[inst->reg]);
	(*reg_ptr) += _tmp;
}

void Bytecode_Interpreter::add_reg (Inst_Add_Reg* inst) {
	assert(INTERP_REGISTER_SIZE == 8);
	auto reg_ptr1 = reinterpret_cast<uint64_t*>(this->registers[inst->reg1]);
	auto reg_ptr2 = reinterpret_cast<uint64_t*>(this->registers[inst->reg2]);
	// TODO: set overflow & underflow flags
	(*reg_ptr1) += (*reg_ptr2);
}

void Bytecode_Interpreter::dump () {
	printf("\n------------ Light VM dump ------------\n\n");
	for (short i = 0; i < INTERP_REGISTER_COUNT; i++) {
        for (size_t j = INTERP_REGISTER_SIZE; j > 0; j--) {
            printf("%02X", this->registers[i][j - 1]);
		}
		if ((i + 1) % 4 == 0) printf("\n");
		else printf("  ");
	}
	printf("\n");
}
