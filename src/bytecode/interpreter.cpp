#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.hpp"

#define DEBUG true

#define INST_OFFSET(offset) buffer[offset]

Bytecode_Interpreter::Bytecode_Interpreter (size_t vm_size) {
	assert(INTERP_REGISTER_SIZE >= sizeof(void*));
	// INFO: not necessary, but makes debug easier
	memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
	this->vm = dcNewCallVM(vm_size);
}

Bytecode_Interpreter::~Bytecode_Interpreter () {
	dcFree(this->vm);
}

void Bytecode_Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
		case BYTECODE_NOOP: return;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);
			if (DEBUG) printf("BYTECODE_COPY\n");
			memcpy(this->registers[cpy->reg1], this->registers[cpy->reg2], INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SET_INTEGER: {
			auto set = static_cast<Inst_Set_Integer*>(inst);
			if (DEBUG) printf("BYTECODE_SET_INTEGER\n");
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
			if (DEBUG) printf("BYTECODE_GLOBAL_OFFSET\n");
			// TODO: get address of global variable (size is irrelevant)
			return;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
			if (DEBUG) printf("BYTECODE_STACK_ALLOCATE\n");
			this->stack_index += alloca->size;
			assert(this->stack_index < INTERP_STACK_SIZE);
			return;
		}
		case BYTECODE_STACK_OFFSET: {
			auto stoff = static_cast<Inst_Stack_Offset*>(inst);
			if (DEBUG) printf("BYTECODE_STACK_OFFSET\n");
			uint8_t* value = this->stack + this->stack_base + stoff->size;
			memcpy(this->registers[stoff->reg], &value, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_LOAD: {
			auto deref = static_cast<Inst_Load*>(inst);
			if (DEBUG) printf("BYTECODE_LOAD\n");
			size_t reg_value = NULL;
			memcpy(&reg_value, this->registers[deref->src], INTERP_REGISTER_SIZE);
			auto mem_ptr = reinterpret_cast<void*>(reg_value);
			memset(this->registers[deref->dest], 0, INTERP_REGISTER_SIZE);
			memcpy(this->registers[deref->dest], mem_ptr, deref->size);
			return;
		}
		case BYTECODE_STORE: {
			auto store = static_cast<Inst_Store*>(inst);
			if (DEBUG) printf("BYTECODE_STORE\n");
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
			if (DEBUG) printf("BYTECODE_ADD\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a + b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SUB: {
			auto add = static_cast<Inst_Sub*>(inst);
			if (DEBUG) printf("BYTECODE_SUB\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a - b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_MUL: {
			auto add = static_cast<Inst_Mul*>(inst);
			if (DEBUG) printf("BYTECODE_MUL\n");
			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a * b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_DIV: {
			auto add = static_cast<Inst_Div*>(inst);
			if (DEBUG) printf("BYTECODE_DIV\n");
			size_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a / b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_CALL_SETUP: {
			auto call_setup = static_cast<Inst_Call_Setup*>(inst);
			if (DEBUG) printf("BYTECODE_CALL_SETUP\n");
			// TODO: setup should set foreign or not to avoid work
			dcMode(vm, DC_CALL_C_X64_WIN64);
			dcReset(vm);
			return;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);
			if (DEBUG) printf("BYTECODE_CALL_PARAM\n");
			// TODO: check next call to see if we really have to do this!
			size_t value;
			memcpy(&value, this->registers[call_param->reg], INTERP_REGISTER_SIZE);
			switch (call_param->bytecode_type) {
				case BYTECODE_TYPE_VOID: break;
				case BYTECODE_TYPE_S8: dcArgChar(vm, (int8_t) value); break;
				case BYTECODE_TYPE_S16: dcArgShort(vm, (int16_t) value); break;
				case BYTECODE_TYPE_S32: dcArgInt(vm, (int32_t) value); break;
				case BYTECODE_TYPE_S64: dcArgLongLong(vm, (int64_t) value); break;
				case BYTECODE_TYPE_U8: dcArgChar(vm, (int8_t) value); break;
				case BYTECODE_TYPE_U16: dcArgShort(vm, (int16_t) value); break;
				case BYTECODE_TYPE_U32: dcArgInt(vm, (int32_t) value); break;
				case BYTECODE_TYPE_U64: dcArgLongLong(vm, (int64_t) value); break;
				case BYTECODE_TYPE_F32: dcArgFloat(vm, (float) value); break;
				case BYTECODE_TYPE_F64: dcArgDouble(vm, (double) value); break;
				case BYTECODE_TYPE_POINTER: dcArgPointer(vm, (char*) value); break;
				case BYTECODE_TYPE_STRING: dcArgPointer(vm, (void*) value); break;
			}
			return;
		}
		case BYTECODE_CALL_FOREIGN: {
			auto call_foreign = static_cast<Inst_Call_Foreign*>(inst);
			if (DEBUG) printf("BYTECODE_CALL_FOREIGN\n");
			auto interp = Light_Compiler::inst->interp;
			auto module_name = interp->foreign_functions->module_names[call_foreign->module_index];
			auto function_name = interp->foreign_functions->function_names[call_foreign->function_index];
			DCpointer function_pointer = interp->foreign_functions->function_pointers[module_name][function_name];

			if (call_foreign->bytecode_type == BYTECODE_TYPE_VOID)
				dcCallVoid(vm, function_pointer);
			else {
				size_t result = 0;
				switch (call_foreign->bytecode_type) {
					case BYTECODE_TYPE_U8:
					case BYTECODE_TYPE_S8:
						result = (size_t) dcCallChar(vm, function_pointer); break;
					case BYTECODE_TYPE_U16:
					case BYTECODE_TYPE_S16:
						result = (size_t) dcCallShort(vm, function_pointer); break;
					case BYTECODE_TYPE_U32:
					case BYTECODE_TYPE_S32:
						result = (size_t) dcCallInt(vm, function_pointer); break;
					case BYTECODE_TYPE_U64:
					case BYTECODE_TYPE_S64:
						result = (size_t) dcCallLongLong(vm, function_pointer); break;
					case BYTECODE_TYPE_F32:
						result = (size_t) dcCallFloat(vm, function_pointer); break;
					case BYTECODE_TYPE_F64:
						result = (size_t) dcCallDouble(vm, function_pointer); break;
					case BYTECODE_TYPE_POINTER:
					case BYTECODE_TYPE_STRING:
						result = (size_t) dcCallPointer(vm, function_pointer); break;
				}
				memcpy(this->registers[call_foreign->reg], &result, INTERP_REGISTER_SIZE);
			}
			return;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			if (DEBUG) printf("BYTECODE_CALL\n");
			return;
		}
		default: {
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

void Bytecode_Interpreter::dump () {
	printf("\n                ----------- Light VM dump ------------\n\n");
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
