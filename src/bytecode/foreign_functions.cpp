#include "bytecode/foreign_functions.hpp"

#include "compiler.hpp"

// TODO: make this platform independent (foreign_functions_win, ...)
#include <windows.h>
#include <stdio.h>

bool Foreign_Functions::store (string module_name, string function_name,
		size_t* module_index, size_t* function_index) {
	HMODULE module = NULL;

	auto module_it = find(this->module_names.begin(), this->module_names.end(), module_name);
	if (module_it == this->module_names.end()) {
		(*module_index) = this->module_names.size();
		this->module_names.push_back(module_name);

		module = LoadLibrary(module_name.c_str());
		if (module) {
			this->module_pointers[module_name] = module;
			//printf("DLL loaded: %s (%p)\n", module_name.c_str(), module);
		} else {
			Light_Compiler::inst->warning(NULL, "DLL not found: '%s'", module_name);
			return false;
		}
	} else {
		module = (HMODULE) this->module_pointers[module_name];
		(*module_index) = distance(this->module_names.begin(), module_it);
	}

	auto function_it = find(this->function_names.begin(), this->function_names.end(), function_name);
	if (function_it == this->function_names.end()) {
		(*function_index) = this->function_names.size();
		this->function_names.push_back(function_name.c_str());

		auto func = GetProcAddress(module, function_name.c_str());
		if (func) {
			this->function_pointers[module_name][function_name] = func;
			//printf("Function loaded: %s (%p) @ %s\n", function_name.c_str(), func, module_name.c_str());
		} else {
			Light_Compiler::inst->warning(NULL, "Function inside DLL not found: '%s'", function_name);
			return false;
		}
	} else {
		(*function_index) = distance(this->function_names.begin(), function_it);
	}

	return true;
}
