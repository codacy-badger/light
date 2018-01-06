#pragma once

#include <stdio.h>
#include <deque>

#include "report.hpp"

#define RING_BUFFER_SIZE 512
#define RING_BUFFER_SECTIONS 4
#define RING_BUFFER_SECTION_SIZE (RING_BUFFER_SIZE / RING_BUFFER_SECTIONS)

struct Ring_Buffer {
	char buffer[RING_BUFFER_SIZE];
	size_t remaining = 0;
	size_t index = 0;
	size_t last = 0;

	FILE* file = NULL;
	Location location;

	Ring_Buffer (FILE* file, const char* filename);

	char next ();
	bool has_next ();
	char peek (size_t offset = 0);
	bool is_next (char c);
	bool is_next (const char* expected);
	void skip (size_t count = 1);
	void skip_any (const char* chars);
	void skip_until (const char* stopper);
	void refill_ring_buffer_if_needed ();
	void handle_location (char character);
};
