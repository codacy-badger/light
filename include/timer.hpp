#pragma once

#include <time.h>

struct Timer {
	static double clockStop (clock_t start);
	static void print (const char* pre, clock_t start);
};
