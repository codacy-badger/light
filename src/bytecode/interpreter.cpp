#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#include "compiler.hpp"
#include "bytecode/primitive_cast.hpp"
#include "bytecode/primitive_unary.hpp"
#include "bytecode/primitive_binary.hpp"

#define DEBUG false

#define INST_OFFSET(offset) buffer[offset]

Bytecode_Interpreter::Bytecode_Interpreter (size_t vm_size) {
	assert(INTERP_REGISTER_SIZE >= sizeof(void*));
	memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
	this->vm = dcNewCallVM(vm_size);
}

Bytecode_Interpreter::~Bytecode_Interpreter () {
	dcFree(this->vm);
}

void Bytecode_Interpreter::run (Ast_Function* func) {
	auto _tmp = this->stack_index;
	for (instruction_index = 0; instruction_index < func->bytecode.size(); instruction_index++) {
		auto inst = func->bytecode[instruction_index];

		if (DEBUG) {
			g_compiler->interp->print(instruction_index, inst);
			if (inst->bytecode == BYTECODE_RETURN || inst->bytecode == BYTECODE_CALL) {
				printf("\n");
			}
		}
		g_compiler->interp->run(inst);
		if (inst->bytecode == BYTECODE_RETURN) {
			//g_compiler->interp->dump();
			break;
		}
	}
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
		case BYTECODE_COPY_MEMORY: {
			auto cpy_mem = static_cast<Inst_Copy_Memory*>(inst);

			size_t val_to, val_from;
			memcpy(&val_to, this->registers[cpy_mem->reg_to], INTERP_REGISTER_SIZE);
			memcpy(&val_from, this->registers[cpy_mem->reg_from], INTERP_REGISTER_SIZE);
			auto ptr_to = reinterpret_cast<void*>(val_to);
			auto ptr_from = reinterpret_cast<void*>(val_from);

			memcpy(ptr_to, ptr_from, cpy_mem->size);
			return;
		}
		case BYTECODE_CAST: {
			auto cast = static_cast<Inst_Cast*>(inst);

			bytecode_cast(this->registers[cast->reg], cast->type_from, cast->type_to);
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

			auto ptr = g_compiler->interp->constants->allocated[coff->offset];
			memcpy(this->registers[coff->reg], &ptr, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);

			auto ptr = g_compiler->interp->globals->get(gloff->offset);
			memcpy(this->registers[gloff->reg], &ptr, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);

			assert((this->stack_index + alloca->size) < INTERP_STACK_SIZE);
			if (alloca->zero_init) {
				auto ptr = this->stack + this->stack_base + this->stack_index;
				memset(ptr, 0, alloca->size);
			}
			this->stack_index += alloca->size;
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
		case BYTECODE_UNARY: {
			auto unary = static_cast<Inst_Unary*>(inst);

			bytecode_unary(unary->unop, this->registers[unary->reg],
				unary->bytecode_type);
			return;
		}
		case BYTECODE_BINARY: {
			auto binary = static_cast<Inst_Binary*>(inst);

			bytecode_binary(binary->binop, this->registers[binary->reg1],
				this->registers[binary->reg2], binary->bytecode_type);
			return;
		}
		case BYTECODE_JUMP: {
			auto jump = static_cast<Inst_Jump*>(inst);
			instruction_index += jump->offset;
			return;
		}
		case BYTECODE_JUMP_IF_FALSE: {
			auto jump_if_true = static_cast<Inst_Jump_If_False*>(inst);
			size_t value;
			memcpy(&value, this->registers[jump_if_true->reg], INTERP_REGISTER_SIZE);
			if (value == 0) instruction_index += jump_if_true->offset;
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

			//printf("\t + Param #%u: %llX (%zd bytes)\n", call_param->index, value, size);
			switch (call_param->bytecode_type) {
				case BYTECODE_TYPE_VOID: break;
				case BYTECODE_TYPE_S8:
				case BYTECODE_TYPE_S16:
				case BYTECODE_TYPE_S32:
				case BYTECODE_TYPE_S64:
				case BYTECODE_TYPE_U8:
				case BYTECODE_TYPE_U16:
				case BYTECODE_TYPE_U32:
				case BYTECODE_TYPE_U64:
				case BYTECODE_TYPE_POINTER: {
					size_t value = 0;
					memcpy(&value, this->registers[call_param->index], size);
					switch (size) {
						case 1: dcArgChar(vm, 		(char) 	value);		break;
						case 2: dcArgShort(vm, 		(short) value);		break;
						case 4: dcArgInt(vm, 		(int) 	value);		break;
						case 8: dcArgLongLong(vm, 	(long long) value);		break;
					}
					break;
				}
				case BYTECODE_TYPE_F32: {
					float value = 0;
					memcpy(&value, this->registers[call_param->index], 4);
					dcArgFloat(vm, value);
					break;
				}
				case BYTECODE_TYPE_F64: {
					double value = 0;
					memcpy(&value, this->registers[call_param->index], 8);
					dcArgDouble(vm, value);
					break;
				}
			}
			return;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);

			//printf("\t ++ Call: %s\n", func->name);
			if (func->foreign_module_name) {
				DCpointer function_pointer = NULL;

				auto ffunctions = g_compiler->interp->foreign_functions;
				auto module = ffunctions->get_or_add_module(func->foreign_module_name);
				if (module) {
					function_pointer = ffunctions->get_or_add_function(module, func->foreign_function_name);
					if (!function_pointer) {
						report_error_stop(&func->location, "Function '%s' not found in module '%s'!",
							func->foreign_function_name, func->foreign_module_name);
					}
				} else {
					report_error_stop(&func->location, "Module '%s' not found!", func->foreign_module_name);
				}

				size_t result = 0;
				auto instance = g_compiler;
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
				auto _inst = this->instruction_index;
				this->stack_base = this->stack_index;
				this->run(func);
				this->stack_index = this->stack_base;
				this->instruction_index = _inst;
				this->stack_base = _base;
			}
			return;
		}
		default: {
			report_error_stop(NULL, "Instruction not yet supported: %d", inst->bytecode);
		}
	}
}

template<typename T>
void _print (void* ptr, const char* printf_format) {
	T value;
	memcpy(&value, ptr, sizeof(T));
	printf(printf_format, value);
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
				case BYTECODE_TYPE_U8:  _print<uint8_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U16: _print<uint16_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U32: _print<uint32_t>(set->data, "%u"); break;
				case BYTECODE_TYPE_U64: _print<uint64_t>(set->data, "%lu"); break;
				case BYTECODE_TYPE_S8:  _print<int8_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S16: _print<int16_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S32: _print<int32_t>(set->data, "%d"); break;
				case BYTECODE_TYPE_S64: _print<int64_t>(set->data, "%ld"); break;
				case BYTECODE_TYPE_F32: _print<float>(set->data, "%f"); break;
				case BYTECODE_TYPE_F64: _print<double>(set->data, "%lf"); break;
				case BYTECODE_TYPE_POINTER: _print<double>(set->data, "0x%llX"); break;
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
		case BYTECODE_STACK_FREE: {
			auto _free = static_cast<Inst_Stack_Free*>(inst);
			printf("STACK_FREE %zd", _free->size);
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
			auto func = reinterpret_cast<Ast_Function*>(call->function_pointer);
			printf("CALL %p (", func);
			if (func->foreign_module_name) {
				printf("%s@%s", func->foreign_function_name, func->foreign_module_name);
			} else printf("%s", func->name);
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
	printf("\nStack [%lld / %d]\n\t", this->stack_index, INTERP_STACK_SIZE);
	for (size_t i = 0; i < this->stack_index; i++) {
		if ((i % 8) == 0) printf("\n");
		printf("%02X ", this->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n");
}
