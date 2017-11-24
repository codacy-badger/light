#pragma once

#include <map>
#include <vector>
#include <string>
#include <assert.h>

// TODO: make this platform independent (foreign_functions_win, ...)
#include <windows.h>

using namespace std;

struct Foreign_Functions {
	map<string, void*> module_pointers;
	map<void*, map<string, void*>> function_pointers;

	void store (string module_name, string function_name) {
		HMODULE module = NULL;

		auto module_it = this->module_pointers.find(module_name);
		if (module_it == this->module_pointers.end()) {
			module = LoadLibrary(module_name.c_str());
			if (module) {
				this->module_pointers[module_name] = module;
			} else assert(!"Handle me!");
		} else {
			module = (HMODULE) this->module_pointers[module_name];
		}

		auto func = GetProcAddress(module, function_name.c_str());
		if (func) {
			this->function_pointers[module][function_name] = func;
		} else assert(!"Handle me!");
	}

	void* get (string module_name, string function_name) {
		auto module_it = this->module_pointers.find(module_name);
		if (module_it != this->module_pointers.end()) {
			auto funcs = this->function_pointers[module_it->second];
			auto func_it = funcs.find(function_name);
			if (func_it != funcs.end()) return func_it->second;
		}
		return NULL;
	}
};
