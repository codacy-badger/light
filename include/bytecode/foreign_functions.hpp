#pragma once

#include <map>
#include <string>

using namespace std;

struct Foreign_Functions {
	map<string, void*> module_pointers;
	map<void*, map<string, void*>> function_pointers;

	void* get_or_add_module (string module_name);
	void* get_or_add_function (void* module, string function_name);
};
