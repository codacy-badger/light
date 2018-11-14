#include "platform.hpp"

#include <stdio.h>
#include <windows.h>

double g_clock_frequency = 0;
HANDLE g_pid = GetCurrentProcess();

uint64_t os_get_wall_time () {
	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return static_cast<uint64_t>(li.QuadPart);
}

uint64_t os_get_user_time () {
	uint64_t user_system_clocks = 0;
	if (QueryProcessCycleTime(g_pid, &user_system_clocks)) {
		return user_system_clocks / 1000;
	} else abort();
}

double os_time_stop (uint64_t start, Timer_Function func) {
	if (g_clock_frequency == 0) {
		LARGE_INTEGER li;
		if(!QueryPerformanceFrequency(&li))
			fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
		else g_clock_frequency = double(li.QuadPart);
	}

    return double(func() - start) / g_clock_frequency;
}

double os_time_wall_stop (uint64_t start) {
	return os_time_stop(start, os_get_wall_time);
}

double os_time_user_stop (uint64_t start) {
	return os_time_stop(start, os_get_user_time);
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

void os_get_absolute_path (const char* relative_path, char* buffer) {
	GetFullPathName(relative_path, MAX_PATH, buffer, NULL);
}

bool os_set_current_directory (const char* new_path) {
	return SetCurrentDirectory(new_path);
}

bool os_set_current_directory_path (const char* new_file_path) {
	auto file_part = os_get_file_part(new_file_path);
	auto tmp = *file_part;
	*file_part = '\0';
	auto result = os_set_current_directory(new_file_path);
	*file_part = tmp;
	return result;
}

char* os_get_file_part (const char* path) {
	auto last_index_of = strrchr(path, '\\');
	if (*(last_index_of + 1) != '\0') {
		return (char*)(last_index_of + 1);
	} else return NULL;
}
