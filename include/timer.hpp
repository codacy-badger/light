#pragma once

#include <stdint.h>
#include <windows.h>
#include <stdio.h>

double PCFreq = 0;

struct Timer {
	static uint64_t getTime () {
		LARGE_INTEGER li;

		if (PCFreq == 0) {
		    if(!QueryPerformanceFrequency(&li))
				fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
			else PCFreq = double(li.QuadPart);
		}

	    QueryPerformanceCounter(&li);
	    return static_cast<uint64_t>(li.QuadPart);
	}

	static double clockStop (uint64_t start) {
		LARGE_INTEGER li;
	    QueryPerformanceCounter(&li);
	    return double(li.QuadPart - start) / PCFreq;
	}

	static void print (const char* pre, uint64_t start) {
		printf("%s%8.6fs\n", pre, Timer::clockStop(start));
	}
};
