#pragma once

#include <stdint.h>
#include <cstdarg>

struct Byte_Buffer {
	size_t size = 0;
	size_t capacity = 0;
	uint8_t* buffer = NULL;

	Byte_Buffer (size_t initial_capacity = 16);
	~Byte_Buffer ();

	void append_1b (uint8_t value);

	void append_2b (uint16_t value);

	void append_4b (uint32_t value);

	void append_8b (uint64_t value);

	void append_bytes (size_t count, ...);
};
