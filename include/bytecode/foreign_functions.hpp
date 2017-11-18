#pragma once

#include <map>
#include <vector>
#include <string>

using namespace std;

struct Foreign_Functions {
	map<string, void*> module_pointers;
	map<string, map<string, void*>> function_pointers;

	void store (string module_name, string function_name);
};
