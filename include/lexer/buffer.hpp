#pragma once

#include <stdio.h>
#include <deque>

#include "report.hpp"

#define RING_BUFFER_SIZE 512
#define RING_BUFFER_SECTIONS 2

struct Buffer {
	int ring_buffer[RING_BUFFER_SIZE];
	int64_t ring_buffer_index = 0;
	int64_t ring_buffer_last = 0;

	FILE* file = NULL;
	Location location;

	Buffer (FILE* file, const char* filename);

	char next ();
	bool has_next ();
	char peek (size_t offset = 0);
	bool is_next (char c);
	bool is_next (const char* expected);
	void skip (size_t count = 1);
	void skip_any (const char* chars);
	void skip_until (const char* stopper);
	void update_ring_buffer_if_needed ();
	void handle_location (char character);
};
