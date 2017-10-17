#pragma once

#include <stdint.h>

struct Timer {
	static uint64_t getTime ();
	static double clockStop (uint64_t start);

	static void print (const char* pre, uint64_t start);
};
