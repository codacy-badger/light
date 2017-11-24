#pragma once

#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#include "compiler.hpp"

#define DEBUG true

#define INST_OFFSET(offset) buffer[offset]

Bytecode_Interpreter::Bytecode_Interpreter (size_t vm_size) {
	assert(INTERP_REGISTER_SIZE >= sizeof(void*));
	memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
	this->vm = dcNewCallVM(vm_size);
}

Bytecode_Interpreter::~Bytecode_Interpreter () {
	dcFree(this->vm);
}

void Bytecode_Interpreter::set (Ast_Comma_Separated_Arguments* args) {
	std::vector<Instruction*> args_setters;
	auto _tmp = this->generator->bytecode;

	this->generator->bytecode = &args_setters;
	for (auto exp : args->values) this->generator->gen(exp);
	for (auto inst : args_setters) this->run(inst);

	this->generator->bytecode = _tmp;
}

void Bytecode_Interpreter::run (Ast_Function* func) {
	auto _tmp = this->stack_index;
	for (size_t i = 0; i < func->bytecode.size(); i++) {
		auto inst = func->bytecode[i];

		if (DEBUG) {
			Light_Compiler::inst->interp->print(i, inst);
			if (inst->bytecode == BYTECODE_RETURN
				|| inst->bytecode == BYTECODE_CALL)
				printf("\n");
		}
		Light_Compiler::inst->interp->run(inst);
		if (inst->bytecode == BYTECODE_RETURN) break;

		//Light_Compiler::inst->interp->dump();
	}
	//Light_Compiler::inst->interp->dump();
	this->stack_index = _tmp;
}

void Bytecode_Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
		case BYTECODE_NOOP: return;
		case BYTECODE_RETURN: return;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);

			memcpy(this->registers[cpy->reg1], this->registers[cpy->reg2], INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_SET: {
			auto set = static_cast<Inst_Set*>(inst);
			auto size = bytecode_get_size(set->bytecode_type);
			memset(this->registers[set->reg], 0, INTERP_REGISTER_SIZE);
			memcpy(this->registers[set->reg], set->data, size);
			return;
		}
		case BYTECODE_CONSTANT_OFFSET: {
			auto coff = static_cast<Inst_Constant_Offset*>(inst);

			auto ptr = Light_Compiler::inst->interp->constants->allocated[coff->offset];
			memcpy(this->registers[coff->reg], &ptr, INTERP_REGISTER_SIZE);
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
		case BYTECODE_STACK_FREE: {
			auto _free = static_cast<Inst_Stack_Free*>(inst);

			this->stack_index -= _free->size;
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
			dcMode(vm, call_setup->calling_convention);
			dcReset(vm);
			return;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);
			auto size = bytecode_get_size(call_param->bytecode_type);

			size_t value = 0;
			memcpy(&value, this->registers[call_param->index], size);
			//printf("\t + Param value: %llX\n", value);
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
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);

			if (func->foreign_module_name) {
				DCpointer function_pointer = NULL;

				auto ffunctions = Light_Compiler::inst->interp->foreign_functions;
				auto module = ffunctions->get_or_add_module(func->foreign_module_name);
				if (module) {
					function_pointer = ffunctions->get_or_add_function(module, func->name);
					if (!function_pointer) {
						Light_Compiler::inst->error_stop(func, "Function '%s' not found (module '%s')!",
							func->name, func->foreign_module_name);
					}
				} else {
					Light_Compiler::inst->error_stop(func, "Module '%s' not found!", func->foreign_module_name);
				}

				size_t result = 0;
				auto instance = Light_Compiler::inst;
				auto ret_ty = func->type->return_type;
				if (ret_ty == instance->type_def_void) {
					dcCallVoid(vm, function_pointer);
				} else if (ret_ty == instance->type_def_s8 || ret_ty == instance->type_def_u8) {
					result = (size_t) dcCallChar(vm, function_pointer);
				} else if (ret_ty == instance->type_def_s16 || ret_ty == instance->type_def_u16) {
					result = (size_t) dcCallShort(vm, function_pointer);
				} else if (ret_ty == instance->type_def_s32 || ret_ty == instance->type_def_u32) {
					result = (size_t) dcCallInt(vm, function_pointer);
				} else if (ret_ty == instance->type_def_s64 || ret_ty == instance->type_def_u64) {
					result = (size_t) dcCallLongLong(vm, function_pointer);
				} else if (ret_ty == instance->type_def_f32) {
					result = (size_t) dcCallFloat(vm, function_pointer);
				} else if (ret_ty == instance->type_def_f64) {
					result = (size_t) dcCallDouble(vm, function_pointer);
				} else {
					result = (size_t) dcCallPointer(vm, function_pointer);
				}
				memcpy(this->registers[0], &result, INTERP_REGISTER_SIZE);
			}else {
				auto _base = this->stack_base;
				this->stack_base = this->stack_index;
				this->run(func);
				this->stack_index = this->stack_base;
				this->stack_base = _base;
			}
			return;
		}
		default: {
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

void Bytecode_Interpreter::print (size_t index, Instruction* inst) {
	printf(" #%-4zd ( %s @ %zd ) ", index, inst->filename, inst->line);
	switch (inst->bytecode) {
		case BYTECODE_NOOP: printf("NOOP"); break;
		case BYTECODE_RETURN: printf("RETURN"); break;
		case BYTECODE_COPY: {
			auto cpy = static_cast<Inst_Copy*>(inst);
			printf("COPY %d, %d", cpy->reg1, cpy->reg2);
			if (cpy->reg1 == cpy->reg2) {
				printf(" [WARNING: instruction has no effect]");
			}
			break;
		}
		case BYTECODE_SET: {
			auto set = static_cast<Inst_Set*>(inst);
			uint64_t value = 0;
			switch (set->bytecode_type) {
				case BYTECODE_TYPE_U8: {
					memcpy(&value, set->data, 1);
					printf("SET %d, %u", set->reg, (uint8_t) value);
					break;
				}
				case BYTECODE_TYPE_U16: {
					memcpy(&value, set->data, 2);
					printf("SET %d, %u", set->reg, (uint16_t) value);
					break;
				}
				case BYTECODE_TYPE_U32: {
					memcpy(&value, set->data, 4);
					printf("SET %d, %u", set->reg, (uint32_t) value);
					break;
				}
				case BYTECODE_TYPE_U64: {
					memcpy(&value, set->data, 8);
					printf("SET %d, %llu", set->reg, (uint64_t) value);
					break;
				}
				case BYTECODE_TYPE_S8: {
					memcpy(&value, set->data, 1);
					printf("SET %d, %d", set->reg, (int8_t) value);
					break;
				}
				case BYTECODE_TYPE_S16: {
					memcpy(&value, set->data, 2);
					printf("SET %d, %d", set->reg, (int16_t) value);
					break;
				}
				case BYTECODE_TYPE_S32: {
					memcpy(&value, set->data, 4);
					printf("SET %d, %d", set->reg, (int32_t) value);
					break;
				}
				case BYTECODE_TYPE_S64: {
					memcpy(&value, set->data, 8);
					printf("SET %d, %lld", set->reg, (int64_t) value);
					break;
				}
				case BYTECODE_TYPE_F32: {
					memcpy(&value, set->data, 4);
					printf("SET %d, %f", set->reg, (float) value);
					break;
				}
				case BYTECODE_TYPE_F64: {
					memcpy(&value, set->data, 8);
					printf("SET %d, %lf", set->reg, (double) value);
					break;
				}
			}
			break;
		}
		case BYTECODE_CONSTANT_OFFSET: {
			auto coff = static_cast<Inst_Constant_Offset*>(inst);
			printf("CONSTANT_OFFSET, %d, %d", coff->reg, coff->offset);
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
		case BYTECODE_STACK_FREE: {
			auto _free = static_cast<Inst_Stack_Free*>(inst);
			printf("STACK_FREE %d", _free->size);
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
			break;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);
			// TODO: print the bytecode type in a readable way
			printf("CALL_PARAM %d, %d", call_param->index, call_param->bytecode_type);
			break;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);
			printf("CALL %p (%s", func, func->name);
			if (func->foreign_module_name) printf("@%s", func->foreign_module_name);
			printf(")");
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
	printf("\n");
}
