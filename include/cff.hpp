#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#define REG_SIZE sizeof(size_t)

typedef void(*cff_function)();

enum CFF_Calling_Convention {
	CFF_CC_UNDEFINED = 0,
	CFF_CC_CDECL,
	CFF_CC_STDCALL,
	CFF_CC_FASTCALL,
};

typedef size_t(__cdecl *cff_cdecl_func0_ret_t)();
typedef size_t(__cdecl *cff_cdecl_func1_ret_t)(size_t);
typedef size_t(__cdecl *cff_cdecl_func2_ret_t)(size_t, size_t);
typedef size_t(__cdecl *cff_cdecl_func3_ret_t)(size_t, size_t, size_t);
typedef size_t(__cdecl *cff_cdecl_func4_ret_t)(size_t, size_t, size_t, size_t);
typedef size_t(__cdecl *cff_cdecl_func5_ret_t)(size_t, size_t, size_t, size_t, size_t);

typedef void(__cdecl *cff_cdecl_func0_t)();
typedef void(__cdecl *cff_cdecl_func1_t)(size_t);
typedef void(__cdecl *cff_cdecl_func2_t)(size_t, size_t);
typedef void(__cdecl *cff_cdecl_func3_t)(size_t, size_t, size_t);
typedef void(__cdecl *cff_cdecl_func4_t)(size_t, size_t, size_t, size_t);
typedef void(__cdecl *cff_cdecl_func5_t)(size_t, size_t, size_t, size_t, size_t);

typedef size_t(__stdcall *cff_stdcall_func0_ret_t)();
typedef size_t(__stdcall *cff_stdcall_func1_ret_t)(size_t);
typedef size_t(__stdcall *cff_stdcall_func2_ret_t)(size_t, size_t);
typedef size_t(__stdcall *cff_stdcall_func3_ret_t)(size_t, size_t, size_t);
typedef size_t(__stdcall *cff_stdcall_func4_ret_t)(size_t, size_t, size_t, size_t);
typedef size_t(__stdcall *cff_stdcall_func5_ret_t)(size_t, size_t, size_t, size_t, size_t);

typedef void(__stdcall *cff_stdcall_func0_t)();
typedef void(__stdcall *cff_stdcall_func1_t)(size_t);
typedef void(__stdcall *cff_stdcall_func2_t)(size_t, size_t);
typedef void(__stdcall *cff_stdcall_func3_t)(size_t, size_t, size_t);
typedef void(__stdcall *cff_stdcall_func4_t)(size_t, size_t, size_t, size_t);
typedef void(__stdcall *cff_stdcall_func5_t)(size_t, size_t, size_t, size_t, size_t);

struct CFF_Function {
	CFF_Calling_Convention calling_convention = CFF_CC_UNDEFINED;
	cff_function function = NULL;
	bool has_return = false;
};

struct CFF_Argument {
    uint8_t* data = NULL;
    size_t size = 0;

	CFF_Argument () {}
    template<typename T> CFF_Argument (T value) : CFF_Argument(&value, sizeof(T)) {}
    CFF_Argument (void* value, size_t size) {
        this->size = size;
        this->data = (uint8_t*) malloc(size);
        memcpy(this->data, value, size);
    }

    size_t deref () {
		if (this->size == 0) return 0;
		else {
			size_t out = 0;
			if (this->size > REG_SIZE) {
				memcpy(&out, &this->data, REG_SIZE);
			}
			else {
				memcpy(&out, this->data, this->size);
			}
			return out;
		}
    }
};

#define ARG(at) (*(argv + at))->deref()
#define CALL_0(func) (func)()
#define CALL_1(func) (func)(ARG(0))
#define CALL_2(func) (func)(ARG(0), ARG(1))
#define CALL_3(func) (func)(ARG(0), ARG(1), ARG(2))
#define CALL_4(func) (func)(ARG(0), ARG(1), ARG(2), ARG(3))
#define CALL_5(func) (func)(ARG(0), ARG(1), ARG(2), ARG(3), ARG(4))

CFF_Function* cff_make_function (cff_function func, bool has_return = false,
        CFF_Calling_Convention cc = CFF_CC_CDECL) {
    auto fn = new CFF_Function();
    fn->calling_convention = cc;
    fn->has_return = has_return;
    fn->function = func;
    return fn;
}

size_t cff_invoke_cdecl_ret (cff_function func, CFF_Argument** argv, size_t argc) {
	assert(argc < 6);
    switch (argc) {
        case 0: return CALL_0((cff_cdecl_func0_ret_t)func);
        case 1: return CALL_1((cff_cdecl_func1_ret_t)func);
        case 2: return CALL_2((cff_cdecl_func2_ret_t)func);
        case 3: return CALL_3((cff_cdecl_func3_ret_t)func);
		case 4: return CALL_4((cff_cdecl_func4_ret_t)func);
        case 5: return CALL_5((cff_cdecl_func5_ret_t)func);
        default: return 0;
    }
}

void cff_invoke_cdecl (cff_function func, CFF_Argument** argv, size_t argc) {
	assert(argc < 6);
    switch (argc) {
        case 0: CALL_0((cff_cdecl_func0_t)func);
        case 1: CALL_1((cff_cdecl_func1_t)func);
        case 2: CALL_2((cff_cdecl_func2_t)func);
        case 3: CALL_3((cff_cdecl_func3_t)func);
        case 4: CALL_4((cff_cdecl_func4_t)func);
        case 5: CALL_5((cff_cdecl_func5_t)func);
        default: return;
    }
}

size_t cff_invoke_stdcall_ret(cff_function func, CFF_Argument** argv, size_t argc) {
	assert(argc < 6);
	switch (argc) {
    	case 0: return CALL_0((cff_stdcall_func0_ret_t)func);
    	case 1: return CALL_1((cff_stdcall_func1_ret_t)func);
    	case 2: return CALL_2((cff_stdcall_func2_ret_t)func);
    	case 3: return CALL_3((cff_stdcall_func3_ret_t)func);
    	case 4: return CALL_4((cff_stdcall_func4_ret_t)func);
    	case 5: return CALL_5((cff_stdcall_func5_ret_t)func);
    	default: return 0;
	}
}

void cff_invoke_stdcall(cff_function func, CFF_Argument** argv, size_t argc) {
	assert(argc < 6);
	switch (argc) {
    	case 0: CALL_0((cff_stdcall_func0_t)func);
    	case 1: CALL_1((cff_stdcall_func1_t)func);
    	case 2: CALL_2((cff_stdcall_func2_t)func);
    	case 3: CALL_3((cff_stdcall_func3_t)func);
    	case 4: CALL_4((cff_stdcall_func4_t)func);
    	case 5: CALL_5((cff_stdcall_func5_t)func);
    	default: return;
	}
}

size_t cff_invoke(CFF_Function* func, CFF_Argument** argv, size_t argc) {
	switch (func->calling_convention) {
        case CFF_CC_UNDEFINED: break;
		case CFF_CC_CDECL: {
			if (!func->has_return) cff_invoke_cdecl(func->function, argv, argc);
			else return cff_invoke_cdecl_ret(func->function, argv, argc);
		}
		case CFF_CC_STDCALL: {
			if (!func->has_return) cff_invoke_stdcall(func->function, argv, argc);
			else return cff_invoke_stdcall_ret(func->function, argv, argc);
		}
		case CFF_CC_FASTCALL: {
			assert(!"fastcall calling convention not yet supported!");
		}
	}
	return 0;
}
