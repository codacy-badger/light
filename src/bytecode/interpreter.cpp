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
}

void Bytecode_Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
		case BYTECODE_NOOP: return;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);
			printf("BYTECODE_COPY\n");
			memcpy(this->registers[cpy->reg1], this->registers[cpy->reg2], INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SET_INTEGER: {
			auto set = static_cast<Inst_Set_Integer*>(inst);
			printf("BYTECODE_SET_INTEGER\n");
			assert(set->size <= INTERP_REGISTER_SIZE);
			memcpy(this->registers[set->reg], set->data, set->size);
			return;
		}
		case BYTECODE_SET_DECIMAL: {
			Light_Compiler::inst->error_stop(NULL, "Decimal bytecode not implemented!");
			return;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);
			printf("BYTECODE_GLOBAL_OFFSET\n");
			// TODO: get address of global variable (size is irrelevant)
			return;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
			printf("BYTECODE_STACK_ALLOCATE\n");
			this->stack_index += alloca->size;
			assert(this->stack_index < INTERP_STACK_SIZE);
			return;
		}
		case BYTECODE_STACK_OFFSET: {
			auto stoff = static_cast<Inst_Stack_Offset*>(inst);
			printf("BYTECODE_STACK_OFFSET\n");
			uint8_t* value = this->stack + this->stack_base + stoff->size;
			memcpy(this->registers[stoff->reg], &value, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_LOAD: {
			auto deref = static_cast<Inst_Load*>(inst);
			printf("BYTECODE_LOAD\n");
			size_t reg_value = NULL;
			memcpy(&reg_value, this->registers[deref->src], INTERP_REGISTER_SIZE);
			auto mem_ptr = reinterpret_cast<void*>(reg_value);
			memset(this->registers[deref->dest], 0, INTERP_REGISTER_SIZE);
			memcpy(this->registers[deref->dest], mem_ptr, deref->size);
			return;
		}
		case BYTECODE_STORE: {
			auto store = static_cast<Inst_Store*>(inst);
			printf("BYTECODE_STORE\n");
			size_t reg_value = NULL;
			memcpy(&reg_value, this->registers[store->dest], INTERP_REGISTER_SIZE);
			auto mem_ptr = reinterpret_cast<void*>(reg_value);
			memcpy(mem_ptr, this->registers[store->src], store->size);
			return;
		}
		case BYTECODE_NOT: {
			return;
		}
		case BYTECODE_NEG: {
			return;
		}
		case BYTECODE_ADD: {
			auto add = static_cast<Inst_Add*>(inst);
			printf("BYTECODE_ADD\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a + b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SUB: {
			auto add = static_cast<Inst_Sub*>(inst);
			printf("BYTECODE_SUB\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a - b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_MUL: {
			auto add = static_cast<Inst_Mul*>(inst);
			printf("BYTECODE_MUL\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a * b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_DIV: {
			auto add = static_cast<Inst_Div*>(inst);
			printf("BYTECODE_DIV\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a / b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		default: {
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

void Bytecode_Interpreter::dump () {
	printf("\n------------ Light VM dump ------------\n\n");
	for (short i = 0; i < INTERP_REGISTER_COUNT; i++) {
        for (size_t j = 0; j < INTERP_REGISTER_SIZE; j++) {
            printf("%02X", this->registers[i][j]);
		}
		if ((i + 1) % 4 == 0) printf("\n");
		else printf("  ");
	}
	printf("\nStack [%lld / %d]\n\n\t", this->stack_index, INTERP_STACK_SIZE);
	for (size_t i = 0; i < this->stack_index; i++) {
		printf("%02X ", this->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n\n");
}
