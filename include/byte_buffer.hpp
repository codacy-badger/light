#pragma once

#include <stdint.h>
#include <cstdarg>

struct Byte_Buffer {
	size_t size = 0;
	size_t capacity = 0;
	uint8_t* buffer = NULL;

	Byte_Buffer (size_t initial_capacity = 16);
	~Byte_Buffer ();

	void append_u8 (uint8_t value);

	void append_u16 (uint16_t value);

	void append_u32 (uint32_t value);

	void append_u64 (uint64_t value);

	void append_bytes (size_t count, ...);
};
