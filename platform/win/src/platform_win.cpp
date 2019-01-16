#include "platform.hpp"

#include <stdio.h>
#include <windows.h>

typedef uint64_t Timer_Function();

#define TARGET_RESOLUTION 1         // 1-millisecond target resolution

double query_clock_frequency () {
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li)) {
		fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
		exit(1);
	} else return double(li.QuadPart);
}

double g_clock_frequency = query_clock_frequency();
HANDLE g_pid = GetCurrentProcess();

OS_Type os_get_type () { return OS_TYPE_WINDOWS; }

Arch_Type os_get_arch () {
	SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
	switch (sysInfo.wProcessorArchitecture) {
		case 9: 	return ARCH_TYPE_X64;
		//case 5: 	return ARCH_TYPE_ARM;
		//case 12: 	return ARCH_TYPE_ARM64;
		//case 6: 	return ARCH_TYPE_IA64;
		//case 0: 	return ARCH_TYPE_X86;
		default: 	return ARCH_TYPE_UNKNOWN;
	}
}

uint64_t os_get_time () {
	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return static_cast<uint64_t>(li.QuadPart);
}

double os_time_stop (uint64_t start) {
	return double(os_get_time() - start) / g_clock_frequency;
}

void os_sleep_for (uint32_t milliseconds) {
	Sleep(milliseconds);
}

void* os_get_module (const char* module_name) {
    return LoadLibrary(module_name);
}

void* os_get_function (void* module, const char* function_name) {
    return GetProcAddress((HMODULE)module, function_name);
}

void* os_get_external_function (const char* module_name, const char* function_name) {
	auto module = os_get_module(module_name);
	if (module) {
		return os_get_function(module, function_name);
	} else return NULL;
}

void os_get_current_directory (char* buffer) {
	GetCurrentDirectory(MAX_PATH, buffer);
}

void os_get_absolute_path (const char* relative_path, char* buffer) {
	GetFullPathName(relative_path, MAX_PATH, buffer, NULL);
}

void os_get_absolute_path_relative_to (const char* relative_path,
		const char* relative_to, char* buffer) {
	char tmp[MAX_PATH];
	os_get_current_directory(tmp);
	os_set_current_directory_path(relative_to);
	GetFullPathName(relative_path, MAX_PATH, buffer, NULL);
	os_set_current_directory(tmp);
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

bool os_check_file_exists (const char* path) {
	WIN32_FIND_DATA FindFileData;
	auto hFind = FindFirstFile(path, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return true;
	} else return false;
}
