#pragma once

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
