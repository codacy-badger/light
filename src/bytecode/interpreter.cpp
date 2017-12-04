#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#include "compiler.hpp"

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
	for (instruction_index = 0; instruction_index < func->bytecode.size(); instruction_index++) {
		auto inst = func->bytecode[instruction_index];

		if (DEBUG) {
			Light_Compiler::inst->interp->print(instruction_index, inst);
			if (inst->bytecode == BYTECODE_RETURN
				|| inst->bytecode == BYTECODE_CALL)
				printf("\n");
		}
		Light_Compiler::inst->interp->run(inst);
		if (inst->bytecode == BYTECODE_RETURN) {
			//Light_Compiler::inst->interp->dump();
			break;
		}
	}
	this->stack_index = _tmp;
}

template<typename F>
void _cast_signed (void* ptr, size_t size_to) {
	F value;
	memcpy(&value, ptr, sizeof(F));
	// We have to do this, since the cast for signed numbers
	// has to mantain the sign bit,
	// TODO: maybe we could simplify this by using bitmasks?
	switch (size_to) {
		case 1: {
			auto new_value = static_cast<int8_t>(value);
			memcpy(ptr, &new_value, size_to);
			break;
		}
		case 2: {
			auto new_value = static_cast<int16_t>(value);
			memcpy(ptr, &new_value, size_to);
			break;
		}
		case 4: {
			auto new_value = static_cast<int32_t>(value);
			memcpy(ptr, &new_value, size_to);
			break;
		}
		case 8: {
			auto new_value = static_cast<int64_t>(value);
			memcpy(ptr, &new_value, size_to);
			break;
		}
	}
}

template<typename F>
void _cast_unsigned (void* ptr, size_t size_to) {
	F value;
	memcpy(&value, ptr, sizeof(F));
	auto new_value = static_cast<size_t>(value);
	memcpy(ptr, &new_value, size_to);
}

void bytecode_cast(void* reg_ptr, uint8_t type_from, uint8_t type_to) {
	auto size_from = bytecode_get_size(type_from);
	auto size_to = bytecode_get_size(type_to);
	auto sign_from = bytecode_has_sign(type_from);

	if (sign_from) {
		switch (size_from) {
			case 1: _cast_signed<int8_t>(reg_ptr, size_to); break;
			case 2: _cast_signed<int16_t>(reg_ptr, size_to); break;
			case 4: _cast_signed<int32_t>(reg_ptr, size_to); break;
			case 8: _cast_signed<int64_t>(reg_ptr, size_to); break;
		}
	} else {
		switch (size_from) {
			case 1: _cast_unsigned<uint8_t>(reg_ptr, size_to); break;
			case 2: _cast_unsigned<uint16_t>(reg_ptr, size_to); break;
			case 4: _cast_unsigned<uint32_t>(reg_ptr, size_to); break;
			case 8: _cast_unsigned<uint64_t>(reg_ptr, size_to); break;
		}
	}
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

			auto ptr = Light_Compiler::inst->interp->constants->allocated[coff->offset];
			memcpy(this->registers[coff->reg], &ptr, INTERP_REGISTER_SIZE);
			return;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);

			auto ptr = Light_Compiler::inst->interp->globals->get(gloff->offset);
			memcpy(this->registers[gloff->reg], &ptr, INTERP_REGISTER_SIZE);
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
		case BYTECODE_UNARY: {
			auto unary = static_cast<Inst_Unary*>(inst);

			size_t a;
			auto size = bytecode_get_size(unary->bytecode_type);
			memcpy(&a, this->registers[unary->reg], size);
			switch (unary->unop) {
				case BYTECODE_LOGICAL_NEGATE: 		a = !a; break;
				case BYTECODE_ARITHMETIC_NEGATE: 	a = -a; break;
				case BYTECODE_BITWISE_NEGATE: 		a = ~a; break;
			}
			memcpy(this->registers[unary->reg], &a, size);
			return;
		}
		case BYTECODE_BINARY: {
			auto binary = static_cast<Inst_Binary*>(inst);

			size_t a, b;
			memcpy(&a, this->registers[binary->reg1], INTERP_REGISTER_SIZE);
			memcpy(&b, this->registers[binary->reg2], INTERP_REGISTER_SIZE);
			switch (binary->binop) {
				case BYTECODE_LOGICAL_AND: 			a = a && b; break;
				case BYTECODE_LOGICAL_OR: 			a = a || b; break;

				case BYTECODE_ADD: 					a = a + b; break;
				case BYTECODE_SUB: 					a = a - b; break;
				case BYTECODE_MUL: 					a = a * b; break;
				case BYTECODE_DIV: 					a = a / b; break;

				case BYTECODE_BITWISE_AND: 			a = a & b; break;
				case BYTECODE_BITWISE_OR: 			a = a | b; break;
				case BYTECODE_BITWISE_XOR: 			a = a ^ b; break;
				case BYTECODE_BITWISE_RIGHT_SHIFT: 	a = a >> b; break;
				case BYTECODE_BITWISE_LEFT_SHIFT: 	a = a << b; break;

				case BYTECODE_EQ: 					a = a == b; break;
				case BYTECODE_NEQ: 					a = a != b; break;
				case BYTECODE_LT: 					a = a < b; break;
				case BYTECODE_LTE: 					a = a <= b; break;
				case BYTECODE_GT: 					a = a > b; break;
				case BYTECODE_GTE: 					a = a >= b; break;
				default:							break;
			}
			memcpy(this->registers[binary->reg1], &a, INTERP_REGISTER_SIZE);
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
					function_pointer = ffunctions->get_or_add_function(module, func->foreign_function_name);
					if (!function_pointer) {
						Light_Compiler::inst->error_stop(func, "Function '%s' not found in module '%s'!",
							func->foreign_function_name, func->foreign_module_name);
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
			Light_Compiler::inst->error_stop(NULL,
				"Instruction not yet supported: %d", inst->bytecode);
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
			printf("JUMP %d", jump->offset);
			break;
		}
		case BYTECODE_JUMP_IF_FALSE: {
			auto jump_if_true = static_cast<Inst_Jump_If_False*>(inst);
			printf("JUMP_IF_FALSE %d, %d", jump_if_true->reg, jump_if_true->offset);
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
	printf("\nStack [%lld / %d]\n\n\t", this->stack_index, INTERP_STACK_SIZE);
	for (size_t i = 0; i < this->stack_index; i++) {
		printf("%02X ", this->stack[i]);
		if ((i + 1) % 32 == 0) printf("\n\t");
	}
	printf("\n");
}
