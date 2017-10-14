#pragma once

#include <iostream>
#include <string>
#include <time.h>

struct Timer {
	static double clockStop (clock_t start) {
		return (clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
	}

	static void print (const char* pre, clock_t start) {
		std::cout << pre << Timer::clockStop(start) << "s\n";
	}
};
