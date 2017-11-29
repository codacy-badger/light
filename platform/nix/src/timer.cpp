#include "timer.hpp"

#include <sys/time.h>

uint64_t Timer::getTime () {
	struct timeval timer;
	gettimeofday(&timer, NULL);
    return static_cast<uint64_t>(timer.tv_usec);
}

double Timer::clockStop (uint64_t start) {
	struct timeval timer;
	gettimeofday(&timer, NULL);
    return double(timer.tv_usec - start) / 1000000.0;
}
