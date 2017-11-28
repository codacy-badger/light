#include "bytecode/foreign_functions.hpp"

#include <assert.h>

#include <windows.h>

void* Foreign_Functions::get_or_add_module (string module_name) {
	auto module_it = this->module_pointers.find(module_name);
	if (module_it == this->module_pointers.end()) {
		auto module = LoadLibrary(module_name.c_str());
		if (module) {
			this->module_pointers[module_name] = module;
		}
		return module;
	} else {
		return this->module_pointers[module_name];
	}
}

void* Foreign_Functions::get_or_add_function (void* module, string function_name) {
	auto function_pointers = this->function_pointers[module];
	auto func_it = function_pointers.find(function_name);
	if (func_it == function_pointers.end()) {
		auto function = GetProcAddress((HMODULE)module, function_name.c_str());
		if (function) {
			this->function_pointers[module][function_name] = function;
		}
		return function;
	} else {
		return function_pointers[function_name];
	}
}

int64_t Foreign_Functions::get_last_error () {
	return GetLastError();
}
