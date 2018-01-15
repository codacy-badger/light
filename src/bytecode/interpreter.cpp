#include "bytecode/interpreter.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.hpp"
#include "bytecode/print.hpp"
#include "bytecode/primitive_cast.hpp"
#include "bytecode/primitive_unary.hpp"
#include "bytecode/primitive_binary.hpp"

//#define BYTECODE_DEBUG

#define IS_INTERNAL_FUNCTION(func) func->stm_type == AST_STATEMENT_EXPRESSION 	\
	&& func->exp_type == AST_EXPRESSION_FUNCTION								\
	&& !func->is_native()														\

#define CLEAR(reg) memset(this->registers[reg], 0, INTERP_REGISTER_SIZE)
#define MOVE(a, b) MOVE_SIZE(a, b, INTERP_REGISTER_SIZE)
#define MOVE_SIZE(a, b, size) memcpy((void*)(a), (void*)(b), size)
#define LOAD_REG(var_name, reg) size_t var_name;								\
	memcpy(&var_name, this->registers[reg], INTERP_REGISTER_SIZE)

Interpreter::Interpreter (size_t vm_size) {
	assert(INTERP_REGISTER_SIZE >= sizeof(void*));
	memset(&this->registers, 0, INTERP_REGISTER_COUNT * sizeof(Bytecode_Register));
	this->vm = dcNewCallVM(vm_size);
}

Interpreter::~Interpreter () {
	dcFree(this->vm);
}

void Interpreter::run (Ast_Function* func) {
	auto _tmp = this->stack_index;
	for (instruction_index = 0; instruction_index < func->bytecode.size(); instruction_index++) {
		auto inst = func->bytecode[instruction_index];

#ifdef BYTECODE_DEBUG
		bytecode_print(instruction_index, inst);
		if (inst->bytecode == BYTECODE_RETURN
				|| inst->bytecode == BYTECODE_CALL) {
			printf("\n");
		}
#endif

		if (inst->bytecode == BYTECODE_RETURN) break;
		else this->run(inst);
	}
	this->stack_index = _tmp;
}

void Interpreter::run (Instruction* inst) {
	switch (inst->bytecode) {
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

			bytecode_cast(this->registers[cast->reg], cast->type_from, cast->type_to);
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
			auto ptr = this->constants->allocated[coff->offset];
			MOVE(this->registers[coff->reg], &ptr);
			return;
		}
		case BYTECODE_GLOBAL_OFFSET: {
			auto gloff = static_cast<Inst_Global_Offset*>(inst);
			auto ptr = this->globals->get(gloff->offset);
			MOVE(this->registers[gloff->reg], &ptr);
			return;
		}
		case BYTECODE_STACK_ALLOCATE: {
			auto alloca = static_cast<Inst_Stack_Allocate*>(inst);
			assert((this->stack_index + alloca->size) < INTERP_STACK_SIZE);
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
			bytecode_unary(unary->unop, this->registers[unary->target],
				this->registers[unary->reg], unary->bytecode_type);
			return;
		}
		case BYTECODE_BINARY: {
			auto binary = static_cast<Inst_Binary*>(inst);
			bytecode_binary(binary->binop, this->registers[binary->target],
				this->registers[binary->reg1], this->registers[binary->reg2],
				binary->bytecode_type);
			return;
		}
		case BYTECODE_ADD_CONST: {
			auto add_c = static_cast<Inst_Add_Const*>(inst);
			LOAD_REG(reg_value, add_c->reg);
			reg_value += add_c->number;
			MOVE(this->registers[add_c->reg], &reg_value);
			return;
		}
		case BYTECODE_MUL_CONST: {
			auto mul_c = static_cast<Inst_Mul_Const*>(inst);
			LOAD_REG(reg_value, mul_c->reg);
			reg_value *= mul_c->number;
			MOVE(this->registers[mul_c->reg], &reg_value);
			return;
		}
		case BYTECODE_JUMP: {
			auto jump = static_cast<Inst_Jump*>(inst);
			instruction_index += jump->offset;
			return;
		}
		case BYTECODE_JUMP_IF_FALSE: {
			auto jump_if_true = static_cast<Inst_Jump_If_False*>(inst);
			LOAD_REG(value, jump_if_true->reg);
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
			LOAD_REG(value, call_param->index);
			switch (call_param->bytecode_type) {
				case BYTECODE_TYPE_BOOL:
				case BYTECODE_TYPE_S8:
				case BYTECODE_TYPE_U8:		dcArgChar(vm, (DCchar) value); break;
				case BYTECODE_TYPE_S16:
				case BYTECODE_TYPE_U16:		dcArgShort(vm, (DCshort) value); break;
				case BYTECODE_TYPE_S32:
				case BYTECODE_TYPE_U32:		dcArgInt(vm, (DCint) value); break;
				case BYTECODE_TYPE_S64:
				case BYTECODE_TYPE_U64:		dcArgLongLong(vm, (DClonglong) value); break;
				case BYTECODE_TYPE_POINTER: dcArgPointer(vm, (DCpointer) value); break;
				case BYTECODE_TYPE_F32: {
					DCfloat tmp;
					assert(sizeof(DCfloat) <= sizeof(size_t));
					memcpy(&tmp, &value, sizeof(DCfloat));
					dcArgFloat(vm, (DCfloat) tmp);
					break;
				}
				case BYTECODE_TYPE_F64: {
					DCdouble tmp;
					assert(sizeof(DCdouble) <= sizeof(size_t));
					memcpy(&tmp, &value, sizeof(DCdouble));
					dcArgDouble(vm, (DCdouble) tmp);
					break;
				}
				default: 					abort();
			}
			return;
		}
		case BYTECODE_CALL: {
			auto call = static_cast<Inst_Call*>(inst);
			LOAD_REG(value, call->reg);
			auto func = reinterpret_cast<Ast_Function*>(value);
			if (IS_INTERNAL_FUNCTION(func)) {
				//printf("\t ++ Call: %s\n", func->name);
				auto _base = this->stack_base;
				auto _inst = this->instruction_index;
				this->stack_base = this->stack_index;
				this->run(func);
				this->stack_index = this->stack_base;
				this->instruction_index = _inst;
				this->stack_base = _base;
			} else {
				auto function_pointer = reinterpret_cast<DCpointer>(value);

				size_t result = 0;
				switch (call->bytecode_type) {
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
				if (call->bytecode_type != BYTECODE_TYPE_VOID) {
					memcpy(this->registers[0], &result, INTERP_REGISTER_SIZE);
				}
			}

			return;
		}
		default: abort();
	}
}
