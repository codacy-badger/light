#pragma once

#include "timer.hpp"

#include <iostream>

double Timer::clockStop (clock_t start) {
	return (clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
}

void Timer::print (const char* pre, clock_t start) {
	std::cout << pre << Timer::clockStop(start) << "s\n";
}
