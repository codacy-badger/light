#pragma once

#include "timer.hpp"

#include <windows.h>
#include <iostream>

double PCFreq = 0;

uint64_t Timer::getTime () {
	LARGE_INTEGER li;

	if (PCFreq == 0) {
	    if(!QueryPerformanceFrequency(&li))
			fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
		else PCFreq = double(li.QuadPart) / 1000.0;
	}

    QueryPerformanceCounter(&li);
    return static_cast<uint64_t>(li.QuadPart);
}

double Timer::clockStop (uint64_t start) {
	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double(li.QuadPart - start) / PCFreq) / 1000.0;
}

void Timer::print (const char* pre, uint64_t start) {
	printf("%s%8.6fs\n", pre, Timer::clockStop(start));
}
