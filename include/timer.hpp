#pragma once

#include <stdint.h>
#include <stdio.h>

struct Timer {
	static uint64_t getTime ();
	static double clockStop (uint64_t start);

	static void print (const char* pre, uint64_t start) {
		printf("%s%8.6fs\n", pre, Timer::clockStop(start));
	}
};
