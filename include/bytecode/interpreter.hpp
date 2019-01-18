#pragma once

#include "platform.hpp"
#include "bytecode/instructions.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"
#include "bytecode/call_record.hpp"

#include "dyncall/dyncall.h"

#include "bytecode/primitive_cast.hpp"
#include "bytecode/primitive_unary.hpp"
#include "bytecode/primitive_binary.hpp"

#include "util/logger.hpp"

#include <assert.h>

#define INTERP_REGISTER_SIZE  sizeof(void*)
#define INTERP_REGISTER_COUNT 16
#define INTERP_STACK_SIZE 	  (1024 * 1024 * 1024) // 1 MB

#define CLEAR(reg) memset(this->registers[reg], 0, INTERP_REGISTER_SIZE)
#define MOVE_SIZE(a, b, size) memcpy((void*)(a), (void*)(b), size)
#define MOVE(a, b) MOVE_SIZE(a, b, INTERP_REGISTER_SIZE)
#define LOAD_REG(var_name, reg) size_t var_name; memcpy(&var_name, this->registers[reg], INTERP_REGISTER_SIZE)

typedef uint8_t Bytecode_Register[INTERP_REGISTER_SIZE];

struct Interpreter {
	Call_Record<Bytecode_Register>* call_record = new Call_Record<Bytecode_Register>();
	Bytecode_Constants* constants = new Bytecode_Constants();
	Bytecode_Globals* globals = new Bytecode_Globals();

	Bytecode_Register registers[INTERP_REGISTER_COUNT];
	uint8_t stack[INTERP_STACK_SIZE];
	size_t instruction_index = 0;
	size_t stack_index = 0;
	size_t stack_base = 0;

	DCCallVM* vm = NULL;

	Interpreter (size_t vm_size = 512) {
		memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
		this->vm = dcNewCallVM(vm_size);
	}

	~Interpreter () { dcFree(this->vm); }

	void run (Ast_Function* func) {
		assert(func->bytecode.size() > 0);

		if (this->call_record) {
			for (uint8_t i = 0; i < this->call_record->param_count; i++) {
				auto param = this->call_record->parameters[i];
				MOVE(this->registers[i], param.value);
			}
		}

		this->run(&func->bytecode);
	}

	void run (std::vector<Instruction*>* instructions) {
		auto _tmp = this->stack_index;
		for (instruction_index = 0; instruction_index < instructions->size(); instruction_index++) {
			auto inst = (*instructions)[instruction_index];

			if (Logger::is_debug())
				this->print(inst);
			this->run(inst);

			if (inst->code == BYTECODE_RETURN) break;
		}
		this->stack_index = _tmp;
	}

	void run (Instruction* inst) {
		switch (inst->code) {
			case BYTECODE_COPY: {
				auto cpy = static_cast<Inst_Copy*>(inst);
				MOVE(this->registers[cpy->reg1], this->registers[cpy->reg2]);
				return;
			}
			case BYTECODE_COPY_MEMORY: {
				auto cpy_mem = static_cast<Inst_Copy_Memory*>(inst);
				LOAD_REG(val_to, cpy_mem->reg_to);
				LOAD_REG(val_from, cpy_mem->reg_from);
				MOVE_SIZE(val_to, val_from, cpy_mem->size);
				return;
			}
			case BYTECODE_CAST: {
				auto cast = static_cast<Inst_Cast*>(inst);

				Bytecode_Primitive_Cast::cast(this->registers[cast->reg_to], this->registers[cast->reg_from],
					cast->type_from, cast->type_to);
				return;
			}
			case BYTECODE_SET: {
				auto set = static_cast<Inst_Set*>(inst);
				auto size = bytecode_get_size(set->bytecode_type);
				CLEAR(set->reg);
				MOVE_SIZE(this->registers[set->reg], set->data, size);
				return;
			}
			case BYTECODE_CONSTANT_OFFSET: {
				auto coff = static_cast<Inst_Constant_Offset*>(inst);
				auto ptr = this->constants->memory[coff->offset];
				MOVE(this->registers[coff->reg], &ptr);
				return;
			}
			case BYTECODE_GLOBAL_OFFSET: {
				auto gloff = static_cast<Inst_Global_Offset*>(inst);
				auto ptr = this->globals->memory[gloff->offset];
				MOVE(this->registers[gloff->reg], &ptr);
				return;
			}
			case BYTECODE_STACK_ALLOCATE: {
				auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
				assert((this->stack_index + alloca->size) < INTERP_STACK_SIZE);

				auto over = this->stack_index % alloca->size;
				if (over > 0) {
					auto padding = alloca->size - over;
					this->stack_index += padding;
				}

				this->stack_index += alloca->size;
				return;
			}
			case BYTECODE_STACK_OFFSET: {
				auto stoff = static_cast<Inst_Stack_Offset*>(inst);
				uint8_t* value = this->stack + this->stack_base + stoff->offset;
				MOVE(this->registers[stoff->reg], &value);
				return;
			}
			case BYTECODE_LOAD: {
				auto deref = static_cast<Inst_Load*>(inst);
				LOAD_REG(reg_value, deref->src);
				CLEAR(deref->dest);
				MOVE_SIZE(this->registers[deref->dest], reg_value, deref->size);
				return;
			}
			case BYTECODE_STORE: {
				auto store = static_cast<Inst_Store*>(inst);
				LOAD_REG(reg_value, store->dest);
				MOVE_SIZE(reg_value, this->registers[store->src], store->size);
				return;
			}
			case BYTECODE_UNARY: {
				auto unary = static_cast<Inst_Unary*>(inst);
				Bytecode_Primitive_Unary::unary(unary->unop, this->registers[unary->target],
					this->registers[unary->reg], unary->bytecode_type);
				return;
			}
			case BYTECODE_BINARY: {
				auto binary = static_cast<Inst_Binary*>(inst);
				Bytecode_Primitive_Binary::binary(binary->binop, this->registers[binary->target],
					this->registers[binary->reg1], this->registers[binary->reg2],
					binary->bytecode_type);
				return;
			}
			case BYTECODE_ADD_CONST: {
				auto add_c = static_cast<Inst_Add_Const*>(inst);
				LOAD_REG(reg_value, add_c->reg);
				reg_value += add_c->number;
				MOVE(this->registers[add_c->target], &reg_value);
				return;
			}
			case BYTECODE_MUL_CONST: {
				auto mul_c = static_cast<Inst_Mul_Const*>(inst);
				LOAD_REG(reg_value, mul_c->reg);
				reg_value *= mul_c->number;
				MOVE(this->registers[mul_c->target], &reg_value);
				return;
			}
			case BYTECODE_JUMP: {
				auto jump = static_cast<Inst_Jump*>(inst);
				instruction_index += jump->offset;
				return;
			}
			case BYTECODE_JUMP_IF_FALSE: {
				auto jump_if_false = static_cast<Inst_Jump_If_False*>(inst);
				size_t value = 0;
				auto size = bytecode_get_size(BYTECODE_TYPE_BOOL);
				memcpy(&value, this->registers[jump_if_false->reg], size);
				if (value == 0) instruction_index += jump_if_false->offset;
				return;
			}
			case BYTECODE_CALL_SETUP: {
				auto call_setup = static_cast<Inst_Call_Setup*>(inst);
				this->call_record->reset();
		        this->call_record->calling_convention = call_setup->calling_convention;
		        this->call_record->param_count = call_setup->param_count;
				return;
			}
			case BYTECODE_RETURN: {
				auto ret = static_cast<Inst_Return*>(inst);
					this->call_record->set_result(ret->bytecode_type,
						&this->registers[ret->reg_index]);
				Logger::verbose("\tReturning value: %lld", *(this->registers[ret->reg_index]));
				return;
			}
			case BYTECODE_CALL_PARAM: {
				auto call_param = static_cast<Inst_Call_Param*>(inst);
				this->call_record->set_param(call_param->param_index,
					call_param->bytecode_type, &this->registers[call_param->reg_index]);
				Logger::verbose("\tParameter value: %lld", *(this->registers[call_param->reg_index]));
				return;
			}
			case BYTECODE_CALL: {
				auto call = static_cast<Inst_Call*>(inst);
				LOAD_REG(value, call->reg_function);
				this->call((void*) value, call->bytecode_type, call->reg_result);
				return;
			}
			case BYTECODE_CALL_CONST: {
				auto call_const = static_cast<Inst_Call_Const*>(inst);
				this->call((void*) call_const->address, call_const->bytecode_type, call_const->reg_result);
				return;
			}
			default: abort();
		}
	}

	void call (void* func_ptr, Bytecode_Type bytecode_type, uint8_t reg_result) {
		auto func = reinterpret_cast<Ast_Function*>(func_ptr);

		if (!func->is_native()) {
			auto _base = this->stack_base;
			auto _inst = this->instruction_index;
			this->stack_base = this->stack_index;

			Bytecode_Register _regs[INTERP_REGISTER_COUNT - 1];
			memcpy(_regs, this->registers, sizeof(_regs));

			Logger::verbose(func, "\tCalling function: '%s'", func->name);
			this->run(func);

			memcpy(this->registers, _regs, sizeof(_regs));

			this->stack_index = this->stack_base;
			this->instruction_index = _inst;
			this->stack_base = _base;

			if (this->call_record->result.bytecode_type != BYTECODE_TYPE_VOID) {
				memcpy(this->registers[reg_result], this->call_record->result.value, INTERP_REGISTER_SIZE);
			}
		} else {
			auto function_pointer = reinterpret_cast<DCpointer>(func_ptr);

			dcMode(vm, this->call_record->calling_convention);
			dcReset(vm);

			for (uint8_t i = 0; i < this->call_record->param_count; i++) {
				auto param = this->call_record->parameters[i];

				size_t param_value;
				memcpy(&param_value, param.value, INTERP_REGISTER_SIZE);

				switch (param.bytecode_type) {
					case BYTECODE_TYPE_BOOL:
					case BYTECODE_TYPE_S8:
					case BYTECODE_TYPE_U8:		dcArgChar(vm, (DCchar) param_value); break;
					case BYTECODE_TYPE_S16:
					case BYTECODE_TYPE_U16:		dcArgShort(vm, (DCshort) param_value); break;
					case BYTECODE_TYPE_S32:
					case BYTECODE_TYPE_U32:		dcArgInt(vm, (DCint) param_value); break;
					case BYTECODE_TYPE_S64:
					case BYTECODE_TYPE_U64:		dcArgLongLong(vm, (DClonglong) param_value); break;
					case BYTECODE_TYPE_POINTER: dcArgPointer(vm, (DCpointer) param_value); break;
					case BYTECODE_TYPE_F32: {
						DCfloat tmp;
						assert(sizeof(DCfloat) <= sizeof(size_t));
						memcpy(&tmp, &param_value, sizeof(DCfloat));
						dcArgFloat(vm, (DCfloat) tmp);
						break;
					}
					case BYTECODE_TYPE_F64: {
						DCdouble tmp;
						assert(sizeof(DCdouble) <= sizeof(size_t));
						memcpy(&tmp, &param_value, sizeof(DCdouble));
						dcArgDouble(vm, (DCdouble) tmp);
						break;
					}
					default: 					abort();
				}
			}

			size_t result = 0;
			switch (bytecode_type) {
				case BYTECODE_TYPE_VOID: 	dcCallVoid(vm, function_pointer); break;
				case BYTECODE_TYPE_U8:
				case BYTECODE_TYPE_S8: 		result = (size_t) dcCallChar(vm, function_pointer); break;
				case BYTECODE_TYPE_U16:
				case BYTECODE_TYPE_S16: 	result = (size_t) dcCallShort(vm, function_pointer); break;
				case BYTECODE_TYPE_U32:
				case BYTECODE_TYPE_S32: 	result = (size_t) dcCallInt(vm, function_pointer); break;
				case BYTECODE_TYPE_U64:
				case BYTECODE_TYPE_S64: 	result = (size_t) dcCallLongLong(vm, function_pointer); break;
				case BYTECODE_TYPE_F32: 	result = (size_t) dcCallFloat(vm, function_pointer); break;
				case BYTECODE_TYPE_F64: 	result = (size_t) dcCallDouble(vm, function_pointer); break;
				case BYTECODE_TYPE_POINTER: result = (size_t) dcCallPointer(vm, function_pointer); break;
			}

			if (bytecode_type != BYTECODE_TYPE_VOID) {
				memcpy(this->registers[reg_result], &result, INTERP_REGISTER_SIZE);
			}
		}
	}

	void dump () {
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

	template<typename T>
	void _bytecode_print (void* ptr, const char* printf_format) {
		T value;
		memcpy(&value, ptr, sizeof(T));
		printf(printf_format, value);
	}

	void print (Instruction* inst) {
		printf("  %s @ %zd ", inst->location.filename, inst->location.line);
		switch (inst->code) {
			case BYTECODE_NOOP: printf("NOOP"); break;
			case BYTECODE_RETURN: {
				auto ret = static_cast<Inst_Return*>(inst);
				printf("RETURN %d, %d\n", ret->reg_index, ret->bytecode_type);
				break;
			}
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
				printf("CAST %d, %d, %d, %d", cast->reg_to, cast->reg_from, cast->type_from, cast->type_to);
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
				printf(" %d, %d, %d", unary->target, unary->reg, unary->bytecode_type);
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
				printf(" %d, %d, %d", binary->target, binary->reg1, binary->reg2);
				break;
			}
			case BYTECODE_ADD_CONST: {
				auto add_c = static_cast<Inst_Add_Const*>(inst);
				printf("ADD_CONSTANT %d, %d, %zd", add_c->target, add_c->reg, add_c->number);
				break;
			}
			case BYTECODE_MUL_CONST: {
				auto mul_c = static_cast<Inst_Mul_Const*>(inst);
				printf("MUL_CONSTANT %d, %d, %zd", mul_c->target, mul_c->reg, mul_c->number);
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
				printf("CALL_SETUP %d, %d", call_setup->param_count, call_setup->calling_convention);
				break;
			}
			case BYTECODE_CALL_PARAM: {
				auto call_param = static_cast<Inst_Call_Param*>(inst);
				printf("CALL_PARAM %d, %d, %d", call_param->param_index, call_param->reg_index, call_param->bytecode_type);
				break;
			}
			case BYTECODE_CALL: {
				auto call = static_cast<Inst_Call*>(inst);
				printf("CALL %d, %d\n", call->reg_result, call->reg_function);
				break;
			}
			case BYTECODE_CALL_CONST: {
				auto call_const = static_cast<Inst_Call_Const*>(inst);
				printf("CALL_CONST %d, 0x%llX\n", call_const->reg_result, call_const->address);
				break;
			}
			default: abort();
		}
		printf("\n");
	}
};
