#pragma once

struct Bytecode_Primitive_Cast {
	template<typename F>
	static void _cast_signed (void* dest, void* ptr, size_t size_to) {
		F value;
		memcpy(&value, ptr, sizeof(F));
		switch (size_to) {
			case 1: {
				auto new_value = static_cast<int8_t>(value);
				memcpy(dest, &new_value, size_to);
				break;
			}
			case 2: {
				auto new_value = static_cast<int16_t>(value);
				memcpy(dest, &new_value, size_to);
				break;
			}
			case 4: {
				auto new_value = static_cast<int32_t>(value);
				memcpy(dest, &new_value, size_to);
				break;
			}
			case 8: {
				auto new_value = static_cast<int64_t>(value);
				memcpy(dest, &new_value, size_to);
				break;
			}
		}
	}

	template<typename F>
	static void _cast_unsigned (void* dest, void* ptr, size_t size_to) {
		F value;
		memcpy(&value, ptr, sizeof(F));
		auto new_value = static_cast<size_t>(value);
		memcpy(dest, &new_value, size_to);
	}

	static void cast(void* dest, void* reg_ptr, Bytecode_Type type_from, Bytecode_Type type_to) {
		auto from_type_size = bytecode_get_size(type_from);
		auto to_type_size = bytecode_get_size(type_to);
		auto from_type_has_sign = bytecode_has_sign(type_from);

		if (type_to == BYTECODE_TYPE_BOOL) {
			uint64_t value = 0;
			memcpy(&value, reg_ptr, from_type_size);
			value = !!value;
			memcpy(dest, &value, 1);
		} else if (from_type_has_sign) {
			switch (from_type_size) {
				case 1: _cast_signed<int8_t>(dest, reg_ptr, to_type_size); break;
				case 2: _cast_signed<int16_t>(dest, reg_ptr, to_type_size); break;
				case 4: _cast_signed<int32_t>(dest, reg_ptr, to_type_size); break;
				case 8: _cast_signed<int64_t>(dest, reg_ptr, to_type_size); break;
			}
		} else {
			switch (from_type_size) {
				case 1: _cast_unsigned<uint8_t>(dest, reg_ptr, to_type_size); break;
				case 2: _cast_unsigned<uint16_t>(dest, reg_ptr, to_type_size); break;
				case 4: _cast_unsigned<uint32_t>(dest, reg_ptr, to_type_size); break;
				case 8: _cast_unsigned<uint64_t>(dest, reg_ptr, to_type_size); break;
			}
		}
	}
};
