#pragma once

template<typename T>
void logical_negate (void* dest, void* ptr) {
	T value;
	memcpy(&value, ptr, sizeof(T));
	value = !value;
	memcpy(dest, &value, sizeof(T));
}

template<typename T, typename R>
void arith_negate (void* dest, void* ptr) {
	T value;
	memcpy(&value, ptr, sizeof(T));
	R result = value;
	result = -result;
	memcpy(dest, &result, sizeof(R));
}

template<typename T>
void arith_negate (void* dest, void* ptr) {
	arith_negate<T, T>(dest, ptr);
}

template<typename T>
void bitwise_negate (void* dest, void* ptr) {
	T value;
	memcpy(&value, ptr, sizeof(T));
	value = ~value;
	memcpy(dest, &value, sizeof(T));
}

void bytecode_unary (uint8_t unop, void* dest, void* ptr, uint8_t bytecode_type) {
	switch (unop) {
		case BYTECODE_LOGICAL_NEGATE: {
			switch (bytecode_type) {
				case BYTECODE_TYPE_BOOL: 	logical_negate<uint8_t>(dest, ptr); break;
				case BYTECODE_TYPE_S8: 		logical_negate<int8_t>(dest, ptr); break;
				case BYTECODE_TYPE_S16: 	logical_negate<int16_t>(dest, ptr); break;
				case BYTECODE_TYPE_S32: 	logical_negate<int32_t>(dest, ptr); break;
				case BYTECODE_TYPE_S64: 	logical_negate<int64_t>(dest, ptr); break;
				case BYTECODE_TYPE_U8: 		logical_negate<uint8_t>(dest, ptr); break;
				case BYTECODE_TYPE_U16: 	logical_negate<uint16_t>(dest, ptr); break;
				case BYTECODE_TYPE_U32: 	logical_negate<uint32_t>(dest, ptr); break;
				case BYTECODE_TYPE_U64: 	logical_negate<uint64_t>(dest, ptr); break;
				case BYTECODE_TYPE_F32: 	logical_negate<float>(dest, ptr); break;
				case BYTECODE_TYPE_F64: 	logical_negate<double>(dest, ptr); break;
				case BYTECODE_TYPE_POINTER: logical_negate<size_t>(dest, ptr); break;
			}
			break;
		}
		case BYTECODE_ARITHMETIC_NEGATE: {
			switch (bytecode_type) {
				case BYTECODE_TYPE_BOOL: 	arith_negate<uint8_t, int16_t>(dest, ptr); break;
				case BYTECODE_TYPE_S8: 		arith_negate<int8_t>(dest, ptr); break;
				case BYTECODE_TYPE_S16: 	arith_negate<int16_t>(dest, ptr); break;
				case BYTECODE_TYPE_S32: 	arith_negate<int32_t>(dest, ptr); break;
				case BYTECODE_TYPE_S64: 	arith_negate<int64_t>(dest, ptr); break;
				case BYTECODE_TYPE_U8: 		arith_negate<uint8_t, int16_t>(dest, ptr); break;
				case BYTECODE_TYPE_U16: 	arith_negate<uint16_t, int32_t>(dest, ptr); break;
				case BYTECODE_TYPE_U32: 	arith_negate<uint32_t, int64_t>(dest, ptr); break;
				case BYTECODE_TYPE_U64: 	arith_negate<uint64_t, int64_t>(dest, ptr); break;
				case BYTECODE_TYPE_F32: 	arith_negate<float>(dest, ptr); break;
				case BYTECODE_TYPE_F64: 	arith_negate<double>(dest, ptr); break;
				case BYTECODE_TYPE_POINTER: arith_negate<size_t, int64_t>(dest, ptr); break;
			}
			break;
		}
		case BYTECODE_BITWISE_NEGATE: {
			switch (bytecode_type) {
				case BYTECODE_TYPE_BOOL: 	bitwise_negate<uint8_t>(dest, ptr); break;
				case BYTECODE_TYPE_S8: 		bitwise_negate<int8_t>(dest, ptr); break;
				case BYTECODE_TYPE_S16: 	bitwise_negate<int16_t>(dest, ptr); break;
				case BYTECODE_TYPE_S32: 	bitwise_negate<int32_t>(dest, ptr); break;
				case BYTECODE_TYPE_S64: 	bitwise_negate<int64_t>(dest, ptr); break;
				case BYTECODE_TYPE_U8: 		bitwise_negate<uint8_t>(dest, ptr); break;
				case BYTECODE_TYPE_U16: 	bitwise_negate<uint16_t>(dest, ptr); break;
				case BYTECODE_TYPE_U32: 	bitwise_negate<uint32_t>(dest, ptr); break;
				case BYTECODE_TYPE_U64: 	bitwise_negate<uint64_t>(dest, ptr); break;
				case BYTECODE_TYPE_POINTER: bitwise_negate<size_t>(dest, ptr); break;
			}
			break;
		}
	}
	return;
}
