#include "bytecode/constants.hpp"

#include <stdio.h>

size_t Bytecode_Constants::add (char* string) {
    this->allocated.push_back((uint8_t*) string);
	return this->allocated.size() - 1;
}
