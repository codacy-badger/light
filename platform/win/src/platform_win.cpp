#include "platform.hpp"

#include <windows.h>

double PCFreq = 0;

uint64_t os_get_time () {
	LARGE_INTEGER li;

	if (PCFreq == 0) {
	    if(!QueryPerformanceFrequency(&li))
			fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
		else PCFreq = double(li.QuadPart);
	}

    QueryPerformanceCounter(&li);
    return static_cast<uint64_t>(li.QuadPart);
}

double os_clock_stop (uint64_t start) {
	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - start) / PCFreq;
}

void* os_get_module (const char* module_name) {
    return LoadLibrary(module_name);
}

void* os_get_function (void* module, const char* function_name) {
    return GetProcAddress((HMODULE)module, function_name);
}

void os_get_current_directory (char* buffer) {
	GetCurrentDirectory(MAX_PATH, buffer);
}

void os_get_absolute_path (const char* relative_path, char* buffer, char** file_part) {
	GetFullPathName(relative_path, MAX_PATH, buffer, file_part);
}

bool os_set_current_directory (char* new_path) {
	return SetCurrentDirectory(new_path);
}
