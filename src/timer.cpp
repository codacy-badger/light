#pragma once

#include "timer.hpp"

#include <iostream>

double Timer::clockStop (clock_t start) {
	return (clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
}

void Timer::print (const char* pre, clock_t start) {
	printf("%s%8.6fs\n", pre, Timer::clockStop(start));
}
