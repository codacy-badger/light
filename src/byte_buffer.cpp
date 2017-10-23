#pragma once

#include "byte_buffer.hpp"

#include <stdlib.h>

Byte_Buffer::Byte_Buffer (size_t initial_capacity) {
	this->buffer = (uint8_t*) malloc(initial_capacity);
	this->capacity = initial_capacity;
	this->size = 0;
}

void Byte_Buffer::resize (size_t new_size) {
	if (new_size > this->capacity)
		realloc(this->buffer, new_size);
}

void Byte_Buffer::append_1b (uint8_t value) {
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_2b (uint16_t value) {
	this->buffer[this->size++] = value >> 8;
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_4b (uint32_t value) {
	this->buffer[this->size++] = value >> 24;
	this->buffer[this->size++] = value >> 16;
	this->buffer[this->size++] = value >> 8;
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_8b (uint64_t value) {
	this->buffer[this->size++] = value >> 56;
	this->buffer[this->size++] = value >> 48;
	this->buffer[this->size++] = value >> 40;
	this->buffer[this->size++] = value >> 32;
	this->buffer[this->size++] = value >> 24;
	this->buffer[this->size++] = value >> 16;
	this->buffer[this->size++] = value >> 8;
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_bytes (size_t count, ...) {
	va_list args;
    va_start(args, count);
	for (size_t i = 0; i < count; i++)
		this->append_1b(va_arg(args, uint8_t));
    va_end(args);
}
