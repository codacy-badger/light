#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.hpp"

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

void Bytecode_Interpreter::run (Ast_Function* func) {
	auto _tmp = this->stack_index;
	for (size_t i = 0; i < func->bytecode.size(); i++) {
		auto inst = func->bytecode[i];

		Light_Compiler::inst->interp->print(i, inst);
		Light_Compiler::inst->interp->run(inst);
		if (inst->bytecode == BYTECODE_RETURN) break;
	}
	//Light_Compiler::inst->interp->dump();
	this->stack_index = _tmp;
}

bool next_call_is_foreign = false;

void Bytecode_Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
		case BYTECODE_NOOP: return;
		case BYTECODE_RETURN: return;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);

			memcpy(this->registers[cpy->reg1], this->registers[cpy->reg2], INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SET_INTEGER: {
			auto set = static_cast<Inst_Set_Integer*>(inst);

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

			// TODO: get address of global variable (size is irrelevant)
			return;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);

			this->stack_index += alloca->size;
			assert(this->stack_index < INTERP_STACK_SIZE);
			return;
		}
		case BYTECODE_STACK_OFFSET: {
			auto stoff = static_cast<Inst_Stack_Offset*>(inst);

			uint8_t* value = this->stack + this->stack_base + stoff->offset;
			memcpy(this->registers[stoff->reg], &value, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_LOAD: {
			auto deref = static_cast<Inst_Load*>(inst);

			size_t reg_value = 0;
			memcpy(&reg_value, this->registers[deref->src], INTERP_REGISTER_SIZE);
			auto mem_ptr = reinterpret_cast<void*>(reg_value);
			memset(this->registers[deref->dest], 0, INTERP_REGISTER_SIZE);
			memcpy(this->registers[deref->dest], mem_ptr, deref->size);
			return;
		}
		case BYTECODE_STORE: {
			auto store = static_cast<Inst_Store*>(inst);

			size_t reg_value = 0;
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

			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a + b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SUB: {
			auto add = static_cast<Inst_Sub*>(inst);

			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a - b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_MUL: {
			auto add = static_cast<Inst_Mul*>(inst);

			int64_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a * b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_DIV: {
			auto add = static_cast<Inst_Div*>(inst);

			size_t a, b;
			memcpy(&a, this->registers[add->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[add->reg2], INTERP_REGISTER_SIZE);
			a = a / b;
			memcpy(this->registers[add->reg1], &a, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_CALL_SETUP: {
			auto call_setup = static_cast<Inst_Call_Setup*>(inst);

			next_call_is_foreign = call_setup->is_foreign;
			if (next_call_is_foreign) {
				dcMode(vm, DC_CALL_C_X64_WIN64);
				dcReset(vm);
			}
			return;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);

			if (next_call_is_foreign) {
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
			} else {
				memcpy(this->registers[call_param->index], this->registers[call_param->reg], INTERP_REGISTER_SIZE);
			}
			return;
		}
		case BYTECODE_CALL_FOREIGN: {
			assert(next_call_is_foreign == true);

			auto call_foreign = static_cast<Inst_Call_Foreign*>(inst);

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
			assert(next_call_is_foreign == false);

			auto call = static_cast<Inst_Call*>(inst);

			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);

			auto _base = this->stack_base;
			this->stack_base = this->stack_index;
			this->run(func);
			this->stack_index = this->stack_base;
			this->stack_base = _base;
			return;
		}
		default: {
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

void Bytecode_Interpreter::print (size_t index, Instruction* inst) {
	printf(" #%-4zd ( %-28s @ %zd ) ", index, inst->filename, inst->line);
	switch (inst->bytecode) {
		case BYTECODE_NOOP: printf("NOOP"); break;
		case BYTECODE_RETURN: printf("RETURN\n"); break;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);
			printf("COPY %d, %d", cpy->reg1, cpy->reg2);
			if (cpy->reg1 == cpy->reg2) {
				printf(" [WARNING: instruction has no effect]");
			}
			break;
		}
		case BYTECODE_SET_INTEGER: {
			auto set_int = static_cast<Inst_Set_Integer*>(inst);
			int64_t value = 0;
			memcpy(&value, set_int->data, set_int->size);
			printf("SET_INTEGER %d, %d, %lld (0x%08llX)", set_int->reg, set_int->size, value, value);
			break;
		}
		case BYTECODE_SET_DECIMAL: {
			//auto cpy = static_cast<Inst_Set_Decimal*>(inst);
			printf("SET_DECIMAL");
			break;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);
			printf("GLOBAL_OFFSET, %d, %d", gloff->reg, gloff->offset);
			break;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
			printf("STACK_ALLOCATE %d", alloca->size);
			break;
		}
		case BYTECODE_STACK_OFFSET: {
			auto stoff = static_cast<Inst_Stack_Offset*>(inst);
			printf("STACK_OFFSET %d, %d", stoff->reg, stoff->offset);
			break;
		}
		case BYTECODE_LOAD: {
			auto load = static_cast<Inst_Load*>(inst);
			printf("LOAD %d, %d, %d", load->dest, load->src, load->size);
			break;
		}
		case BYTECODE_STORE: {
			auto store = static_cast<Inst_Store*>(inst);
			printf("STORE %d, %d, %d", store->dest, store->src, store->size);
			break;
		}
		case BYTECODE_NOT: {
			//auto cpy = static_cast<Inst_Not*>(inst);
			printf("NOT");
			break;
		}
		case BYTECODE_NEG: {
			//auto cpy = static_cast<Inst_Neg*>(inst);
			printf("NEG");
			break;
		}
		case BYTECODE_ADD: {
			auto add = static_cast<Inst_Add*>(inst);
			printf("ADD %d, %d", add->reg1, add->reg2);
			break;
		}
		case BYTECODE_SUB: {
			auto sub = static_cast<Inst_Sub*>(inst);
			printf("SUB %d, %d", sub->reg1, sub->reg2);
			break;
		}
		case BYTECODE_MUL: {
			auto mul = static_cast<Inst_Mul*>(inst);
			printf("MUL %d, %d", mul->reg1, mul->reg2);
			break;
		}
		case BYTECODE_DIV: {
			auto div = static_cast<Inst_Div*>(inst);
			printf("DIV %d, %d", div->reg1, div->reg2);
			break;
		}
		case BYTECODE_CALL_SETUP: {
			auto call_setup = static_cast<Inst_Call_Setup*>(inst);
			printf("CALL_SETUP %d", call_setup->calling_convention);
			switch (call_setup->calling_convention) {
				case BYTECODE_CC_DEFAULT:  printf(" (DEFAULT)"); break;
				case BYTECODE_CC_CDECL:    printf(" (CDECL)"); break;
				case BYTECODE_CC_STDCALL:  printf(" (STDCALL)"); break;
				case BYTECODE_CC_FASTCALL: printf(" (FASTCALL)"); break;
			}
			printf(", %d", call_setup->is_foreign);
			if (call_setup->is_foreign) printf(" (external)");
			else printf(" (internal)");
			break;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);
			// TODO: print the bytecode type in a readable way
			printf("CALL_PARAM %d, %d, %d", call_param->index, call_param->reg, call_param->bytecode_type);
			break;
		}
		case BYTECODE_CALL_FOREIGN: {
			auto call_f = static_cast<Inst_Call_Foreign*>(inst);
			auto module_name = Light_Compiler::inst->interp->foreign_functions->module_names[call_f->module_index];
			auto function_name = Light_Compiler::inst->interp->foreign_functions->function_names[call_f->function_index];
			printf("CALL_FOREIGN %d, %d (%s), %d (%s), %d\n", call_f->reg, call_f->module_index,
				module_name.c_str(), call_f->function_index, function_name.c_str(), call_f->bytecode_type);
			break;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);
			printf("CALL %d, %p (%s)\n", call->reg, func, func->name);
			break;
		}
		default: assert(false);
	}
	printf("\n");
}

void Bytecode_Interpreter::dump () {
	printf("\n                ----------- Light VM dump ------------\n\n");
	for (short i = 0; i < INTERP_REGISTER_COUNT; i++) {
        for (size_t j = INTERP_REGISTER_SIZE; j > 0; j--) {
            printf("%02X", this->registers[i][j - 1]);
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
