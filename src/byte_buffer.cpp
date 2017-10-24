#pragma once

#include "byte_buffer.hpp"

#include <stdio.h>
#include <stdlib.h>

void resize_if_needed (Byte_Buffer* bb, int ahead = 0) {
	while ((bb->size + ahead) >= bb->capacity) {
		bb->capacity *= 2;
		auto _tmp = (uint8_t*) realloc(bb->buffer, bb->capacity);
		if (_tmp) {
			//printf("[Byte_Buffer] realloc! %lld\n", bb->capacity);
			bb->buffer = _tmp;
		} else {
			printf("[Byte_Buffer] ERROR: realloc failed!\n");
		}
	}
}

Byte_Buffer::Byte_Buffer (size_t initial_capacity) {
	this->buffer = (uint8_t*) malloc(initial_capacity);
	this->capacity = initial_capacity;
	this->size = 0;
}

Byte_Buffer::~Byte_Buffer () {
	free(this->buffer);
}

void Byte_Buffer::append_u8 (uint8_t value) {
	resize_if_needed(this, 1);
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_u16 (uint16_t value) {
	resize_if_needed(this, 2);
	this->buffer[this->size++] = value >> 8;
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_u32 (uint32_t value) {
	resize_if_needed(this, 4);
	this->buffer[this->size++] = value >> 24;
	this->buffer[this->size++] = value >> 16;
	this->buffer[this->size++] = value >> 8;
	this->buffer[this->size++] = value;
}

void Byte_Buffer::append_u64 (uint64_t value) {
	resize_if_needed(this, 8);
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
	resize_if_needed(this, count);
	va_list args;
    va_start(args, count);
	for (size_t i = 0; i < count; i++)
		this->append_u8(va_arg(args, uint8_t));
    va_end(args);
}
