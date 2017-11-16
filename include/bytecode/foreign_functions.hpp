#pragma once

#include <map>
#include <vector>
#include <string>

using namespace std;

struct Foreign_Functions {
	map<string, void*> module_pointers;
	map<string, map<string, void*>> function_pointers;
	vector<string> module_names;
	vector<string> function_names;

	bool store (string module_name, string function_name, size_t* module_index, size_t* function_index);
};
