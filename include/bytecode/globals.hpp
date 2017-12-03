#pragma once

#include <stdio.h>
#include <map>

struct Global_Info {
	void* pointer = NULL;
	size_t size = 0;
};

struct Bytecode_Globals {
    std::map<size_t, Global_Info*> allocated;
	size_t current_offset = 0;

	size_t add (size_t size) {
		auto global_info = new Global_Info();
		global_info->size = size;
		auto _offset = this->current_offset;
		this->allocated[_offset] = global_info;
		this->current_offset += size;
		return _offset;
	}

    void* get (size_t offset) {
        auto it = this->allocated.find(offset);
        if (it == this->allocated.end()) return NULL;
        else {
			if (it->second->pointer == NULL) {
	            printf("Allocating global variable at %zd (size %zd)\n", offset, it->second->size);
	            auto ptr = malloc(it->second->size);
	            this->allocated[offset]->pointer = ptr;
				return ptr;
			} else return it->second->pointer;
        }
    }
};
