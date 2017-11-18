#include "bytecode/foreign_functions.hpp"

#include "compiler.hpp"

// TODO: make this platform independent (foreign_functions_win, ...)
#include <windows.h>
#include <stdio.h>

void Foreign_Functions::store (string module_name, string function_name) {
	HMODULE module = NULL;

	auto module_it = this->module_pointers.find(module_name);
	if (module_it == this->module_pointers.end()) {
		module = LoadLibrary(module_name.c_str());
		if (module) {
			this->module_pointers[module_name] = module;
		} else {
			Light_Compiler::inst->error_stop(NULL, "DLL not found: '%s'", module_name);
		}
	} else {
		module = (HMODULE) this->module_pointers[module_name];
	}

	auto function_it = this->function_pointers.find(function_name);
	if (function_it == this->function_pointers.end()) {
		auto func = GetProcAddress(module, function_name.c_str());
		if (func) {
			this->function_pointers[module_name][function_name] = func;
		} else {
			Light_Compiler::inst->error_stop(NULL, "Function inside DLL not found: '%s'", function_name);
		}
	}
}
