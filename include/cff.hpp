#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

typedef size_t (__cdecl *cff_cdecl_func_ret_t)(...);
typedef void (__cdecl *cff_cdecl_func_t)(...);

typedef size_t (__stdcall *cff_stdcall_func_ret_t)(...);
typedef void (__stdcall *cff_stdcall_func_t)(...);

typedef size_t (__fastcall *cff_fastcall_func_ret_t)(...);
typedef void (__fastcall *cff_fastcall_func_t)(...);

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

    size_t deref () {
        size_t out = 0;
        memcpy(&out, &this->data, this->size);
        return out;
    }
};

#define ARG(at) (*(argv + at))->deref()

#define CALL_0(func) func()
#define CALL_1(func) func(ARG(0))
#define CALL_2(func) func(ARG(0), ARG(1))
#define CALL_3(func) func(ARG(0), ARG(1), ARG(2))
#define CALL_4(func) func(ARG(0), ARG(1), ARG(2), ARG(3))
#define CALL_5(func) func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4))
#define CALL_6(func) func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5))
#define CALL_7(func) func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6))
#define CALL_8(func) func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6), ARG(7))
#define CALL_9(func) func(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6), ARG(7), ARG(8))

#define CALLS_RET(argc, func)                                           \
    assert(argc < 10);                                                  \
    switch (argc) {                                                     \
        case 0: return CALL_0(func);                                    \
        case 1: return CALL_1(func);                                    \
        case 2: return CALL_2(func);                                    \
        case 3: return CALL_3(func);                                    \
        case 4: return CALL_4(func);                                    \
        case 5: return CALL_5(func);                                    \
        case 6: return CALL_6(func);                                    \
        case 7: return CALL_7(func);                                    \
        case 8: return CALL_8(func);                                    \
        case 9: return CALL_9(func);                                    \
        default: return NULL;                                           \
    }

#define CALLS_VOID(argc, func)                                          \
    assert(argc < 10);                                                  \
    switch (argc) {                                                     \
        case 0: CALL_0(func); return;                                   \
        case 1: CALL_1(func); return;                                   \
        case 2: CALL_2(func); return;                                   \
        case 3: CALL_3(func); return;                                   \
        case 4: CALL_4(func); return;                                   \
        case 5: CALL_5(func); return;                                   \
        case 6: CALL_6(func); return;                                   \
        case 7: CALL_7(func); return;                                   \
        case 8: CALL_8(func); return;                                   \
        case 9: CALL_9(func); return;                                   \
        default: return;                                                \
    }

size_t cff_invoke_cdecl (cff_cdecl_func_ret_t func, CFF_Argument** argv, size_t argc) {
    CALLS_RET(argc, func)
}

void cff_invoke_cdecl (cff_cdecl_func_t func, CFF_Argument** argv, size_t argc) {
    CALLS_VOID(argc, func)
}

size_t cff_invoke_stdcall (cff_stdcall_func_ret_t func, CFF_Argument** argv, size_t argc) {
    CALLS_RET(argc, func)
}

void cff_invoke_stdcall (cff_stdcall_func_t func, CFF_Argument** argv, size_t argc) {
    CALLS_VOID(argc, func)
}

size_t cff_invoke_fastcall (cff_fastcall_func_ret_t func, CFF_Argument** argv, size_t argc) {
    CALLS_RET(argc, func)
}

void cff_invoke_fastcall (cff_fastcall_func_t func, CFF_Argument** argv, size_t argc) {
    CALLS_VOID(argc, func)
}
