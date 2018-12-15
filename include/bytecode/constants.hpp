#pragma once

#include <vector>

using namespace std;

struct Bytecode_Constants {
    vector<void*> allocated;

    size_t add (const char* string) {
        this->allocated.push_back((void*) string);
    	return this->allocated.size() - 1;
    }
};
