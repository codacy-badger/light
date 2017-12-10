#pragma once

template<typename T>
void binary_result (uint8_t binop, void* a_ptr, void* b_ptr) {
	T a, b;
	int8_t r = -1;
	memcpy(&a, a_ptr, sizeof(T));
	memcpy(&b, b_ptr, sizeof(T));
	switch (binop) {
		case BYTECODE_LOGICAL_AND: 			r = a && b; break;
		case BYTECODE_LOGICAL_OR: 			r = a || b; break;

		case BYTECODE_ADD: 					a = a + b; break;
		case BYTECODE_SUB: 					a = a - b; break;
		case BYTECODE_MUL: 					a = a * b; break;
		case BYTECODE_DIV: 					a = a / b; break;

		case BYTECODE_EQ: 					r = a == b; break;
		case BYTECODE_NEQ: 					r = a != b; break;
		case BYTECODE_LT: 					r = a < b; break;
		case BYTECODE_LTE: 					r = a <= b; break;
		case BYTECODE_GT: 					r = a > b; break;
		case BYTECODE_GTE: 					r = a >= b; break;
	}
	if (r >= 0) memcpy(a_ptr, &r, 1);
	else 		memcpy(a_ptr, &a, sizeof(T));
}

template<typename T>
void binary_result_logic (uint8_t binop, void* a_ptr, void* b_ptr) {
	T a, b;
	memcpy(&a, a_ptr, sizeof(T));
	memcpy(&b, b_ptr, sizeof(T));
	switch (binop) {
		case BYTECODE_LOGICAL_AND: 			a = a && b; break;
		case BYTECODE_LOGICAL_OR: 			a = a || b; break;

		case BYTECODE_ADD: 					a = a + b; break;
		case BYTECODE_SUB: 					a = a - b; break;
		case BYTECODE_MUL: 					a = a * b; break;
		case BYTECODE_DIV: 					a = a / b; break;
		case BYTECODE_REM: 					a = a % b; break;

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
	}
	memcpy(a_ptr, &a, sizeof(T));
}

void bytecode_binary (uint8_t binop, void* a_ptr, void* b_ptr, uint8_t bytecode_type) {
	switch (bytecode_type) {
		case BYTECODE_TYPE_BOOL: 	binary_result_logic<uint8_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_S8: 		binary_result_logic<int8_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_S16: 	binary_result_logic<int16_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_S32: 	binary_result_logic<int32_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_S64: 	binary_result_logic<int64_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_U8: 		binary_result_logic<uint8_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_U16: 	binary_result_logic<uint16_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_U32: 	binary_result_logic<uint32_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_U64: 	binary_result_logic<uint64_t>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_F32: 	binary_result<float>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_F64: 	binary_result<double>(binop, a_ptr, b_ptr); break;
		case BYTECODE_TYPE_POINTER: binary_result_logic<uint64_t>(binop, a_ptr, b_ptr); break;
	}
	return;
}
