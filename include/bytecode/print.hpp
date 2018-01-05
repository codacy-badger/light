#pragma once

#include <string.h>
#include "bytecode/interpreter.hpp"

template<typename T>
void _bytecode_print (void* ptr, const char* printf_format) {
	T value;
	memcpy(&value, ptr, sizeof(T));
	printf(printf_format, value);
}

void bytecode_print (size_t index, Instruction* inst) {
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
		case BYTECODE_COPY_MEMORY: {
			auto cpy_mem = static_cast<Inst_Copy_Memory*>(inst);
			printf("COPY_MEMORY %d, %d, %zd", cpy_mem->reg_to,
				cpy_mem->reg_from, cpy_mem->size);
			break;
		}
		case BYTECODE_CAST: {
			auto cast = static_cast<Inst_Cast*>(inst);
			printf("CAST %d, %d, %d", cast->reg, cast->type_from, cast->type_to);
			break;
		}
		case BYTECODE_SET: {
			auto set = static_cast<Inst_Set*>(inst);
			printf("SET %d, ", set->reg);
			switch (set->bytecode_type) {
				case BYTECODE_TYPE_U8:      _bytecode_print<uint8_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U16:     _bytecode_print<uint16_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U32:     _bytecode_print<uint32_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U64:     _bytecode_print<uint64_t>(set->data, "%lu"); break;
				case BYTECODE_TYPE_S8:      _bytecode_print<int8_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S16:     _bytecode_print<int16_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S32:     _bytecode_print<int32_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S64:     _bytecode_print<int64_t>(set->data, "%ld"); break;
				case BYTECODE_TYPE_F32:     _bytecode_print<float>(set->data, "%f"); break;
				case BYTECODE_TYPE_F64:     _bytecode_print<double>(set->data, "%lf"); break;
				case BYTECODE_TYPE_POINTER: _bytecode_print<double>(set->data, "0x%llX"); break;
			}
			break;
		}
		case BYTECODE_CONSTANT_OFFSET: {
			auto coff = static_cast<Inst_Constant_Offset*>(inst);
			printf("CONSTANT_OFFSET, %d, %zd", coff->reg, coff->offset);
			break;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);
			printf("GLOBAL_OFFSET, %d, %zd", gloff->reg, gloff->offset);
			break;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
			printf("STACK_ALLOCATE %zd", alloca->size);
			break;
		}
		case BYTECODE_STACK_OFFSET: {
			auto stoff = static_cast<Inst_Stack_Offset*>(inst);
			printf("STACK_OFFSET %d, %zd", stoff->reg, stoff->offset);
			break;
		}
		case BYTECODE_LOAD: {
			auto load = static_cast<Inst_Load*>(inst);
			printf("LOAD %d, %d, %zd", load->dest, load->src, load->size);
			break;
		}
		case BYTECODE_STORE: {
			auto store = static_cast<Inst_Store*>(inst);
			printf("STORE %d, %d, %zd", store->dest, store->src, store->size);
			break;
		}
		case BYTECODE_UNARY: {
			auto unary = static_cast<Inst_Unary*>(inst);
			switch (unary->unop) {
				case BYTECODE_LOGICAL_NEGATE: 		printf("LOGICAL_NEGATE"); break;
				case BYTECODE_ARITHMETIC_NEGATE: 	printf("ARITHMETIC_NEGATE"); break;
				case BYTECODE_BITWISE_NEGATE: 		printf("BITWISE_NEGATE"); break;
			}
			printf(" %d, %d", unary->reg, unary->bytecode_type);
			break;
		}
		case BYTECODE_BINARY: {
			auto binary = static_cast<Inst_Binary*>(inst);
			switch (binary->binop) {
				case BYTECODE_LOGICAL_AND: 			printf("LOGICAL_AND"); break;
				case BYTECODE_LOGICAL_OR: 			printf("LOGICAL_OR"); break;

				case BYTECODE_ADD: 					printf("ADD"); break;
				case BYTECODE_SUB: 					printf("SUB"); break;
				case BYTECODE_MUL: 					printf("MUL"); break;
				case BYTECODE_DIV: 					printf("DIV"); break;
				case BYTECODE_REM: 					printf("REM"); break;

				case BYTECODE_BITWISE_AND: 			printf("BITWISE_AND"); break;
				case BYTECODE_BITWISE_OR: 			printf("BITWISE_OR"); break;
				case BYTECODE_BITWISE_XOR: 			printf("BITWISE_XOR"); break;
				case BYTECODE_BITWISE_RIGHT_SHIFT: 	printf("BITWISE_RIGHT_SHIFT"); break;
				case BYTECODE_BITWISE_LEFT_SHIFT: 	printf("BITWISE_LEFT_SHIFT"); break;

				case BYTECODE_EQ: 					printf("EQ"); break;
				case BYTECODE_NEQ: 					printf("NEQ"); break;
				case BYTECODE_LT: 					printf("LT"); break;
				case BYTECODE_LTE: 					printf("LTE"); break;
				case BYTECODE_GT: 					printf("GT"); break;
				case BYTECODE_GTE: 					printf("GTE"); break;
			}
			printf(" %d, %d", binary->reg1, binary->reg2);
			break;
		}
		case BYTECODE_ADD_CONST: {
			auto add_c = static_cast<Inst_Add_Const*>(inst);
			printf("ADD_CONSTANT %d, %zd", add_c->reg, add_c->number);
			break;
		}
		case BYTECODE_MUL_CONST: {
			auto mul_c = static_cast<Inst_Mul_Const*>(inst);
			printf("MUL_CONSTANT %d, %zd", mul_c->reg, mul_c->number);
			break;
		}
		case BYTECODE_JUMP: {
			auto jump = static_cast<Inst_Jump*>(inst);
			printf("JUMP %zd", jump->offset);
			break;
		}
		case BYTECODE_JUMP_IF_FALSE: {
			auto jump_if_true = static_cast<Inst_Jump_If_False*>(inst);
			printf("JUMP_IF_FALSE %d, %zd", jump_if_true->reg, jump_if_true->offset);
			break;
		}
		case BYTECODE_CALL_SETUP: {
			auto call_setup = static_cast<Inst_Call_Setup*>(inst);
			printf("CALL_SETUP %d", call_setup->calling_convention);
			break;
		}
		case BYTECODE_CALL_PARAM: {
			auto call_param = static_cast<Inst_Call_Param*>(inst);
			printf("CALL_PARAM %d, %d", call_param->index, call_param->bytecode_type);
			break;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			printf("CALL %d", call->reg);
			break;
		}
		default: abort();
	}
	printf("\n");
}

void bytecode_dump (Interpreter* interp) {
	printf("\n                ----------- Light VM dump ------------\n\n");
	for (short i = 0; i < INTERP_REGISTER_COUNT; i++) {
        for (size_t j = INTERP_REGISTER_SIZE; j > 0; j--) {
            printf("%02X", interp->registers[i][j - 1]);
		}
		if ((i + 1) % 4 == 0) printf("\n");
		else printf("  ");
	}
	printf("\nStack [%lld / %d]\n\t", interp->stack_index, INTERP_STACK_SIZE);
	for (size_t i = 0; i < interp->stack_index; i++) {
		if ((i % 8) == 0) printf("\n");
		printf("%02X ", interp->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n");
}
