#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

typedef uint64_t (*cff_func_ret_t)(...);
typedef void (*cff_func_t)(...);

struct CFF_Argument {
    uint8_t* data;
    size_t size;

    CFF_Argument (void* value)    : CFF_Argument(&value, sizeof(void*)) {}
    CFF_Argument (bool value)     : CFF_Argument(&value, sizeof(bool)) {}
    CFF_Argument (char* value)    : CFF_Argument(&value, sizeof(char*)) {}
    CFF_Argument (uint8_t value)  : CFF_Argument(&value, sizeof(uint8_t)) {}
    CFF_Argument (uint16_t value) : CFF_Argument(&value, sizeof(uint16_t)) {}
    CFF_Argument (uint32_t value) : CFF_Argument(&value, sizeof(uint32_t)) {}
    CFF_Argument (uint64_t value) : CFF_Argument(&value, sizeof(uint64_t)) {}

    CFF_Argument (void* value, size_t size) {
        this->size = size;
        this->data = (uint8_t*) malloc(size);
        memcpy(&this->data, value, size);
    }

    uint64_t deref () {
        uint64_t out = 0;
        memcpy(&out, &this->data, this->size);
        return out;
    }
};

#define ARG(at) (*(argv + at))->deref()

uint64_t cff_invoke (cff_func_ret_t func, CFF_Argument** argv, size_t argc) {
    assert(argc <= 6);
    switch (argc) {
        case 0: return func();
        case 1: return func(ARG(0));
        case 2: return func(ARG(0), ARG(1));
        case 3: return func(ARG(0), ARG(1), ARG(2));
        case 4: return func(ARG(0), ARG(1), ARG(2), ARG(3));
        case 5: return func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4));
        case 6: return func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5));
        default: return NULL;
    }
}

void cff_invoke (cff_func_t func, CFF_Argument** argv, size_t argc) {
    assert(argc <= 6);
    switch (argc) {
        case 0: func(); return;
        case 1: func(ARG(0)); return;
        case 2: func(ARG(0), ARG(1)); return;
        case 3: func(ARG(0), ARG(1), ARG(2)); return;
        case 4: func(ARG(0), ARG(1), ARG(2), ARG(3)); return;
        case 5: func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4)); return;
        case 6: func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5)); return;
        default: return;
    }
}
