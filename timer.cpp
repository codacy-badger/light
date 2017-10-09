#pragma once

#include <iostream>
#include <string>

struct Timer {
	static double clockStop (clock_t start) {
		return (clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
	}

	static void print (std::string pre, clock_t start) {
		std::cout << pre << Timer::clockStop(start) << "s\n";
	}
};
