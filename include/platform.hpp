#pragma once

#include <stdint.h>

#define MAX_PATH_LENGTH 260

typedef uint64_t Timer_Function();

enum OS_Type : uint8_t {
    OS_TYPE_UNKNOWN = 0,

    OS_TYPE_WINDOWS,
    OS_TYPE_LINUX,
    OS_TYPE_MAC,
};

enum Arch_Type : uint8_t {
    ARCH_TYPE_UNKNOWN = 0,

    ARCH_TYPE_X64,
};

OS_Type os_get_type ();
Arch_Type os_get_arch ();

uint64_t os_get_wall_time ();
uint64_t os_get_user_time ();
double os_time_stop (uint64_t start, Timer_Function func);
double os_time_wall_stop (uint64_t start);
double os_time_user_stop (uint64_t start);

void* os_get_module (const char* module_name);
void* os_get_function (void* module, const char* function_name);
void* os_get_external_function (const char* module_name, const char* function_name);

void os_get_current_directory (char* buffer);
void os_get_absolute_path (const char* relative_path, char* buffer);
void os_get_absolute_path_relative_to (const char* relative_path, const char* relative_to, char* buffer);
bool os_set_current_directory (const char* new_path);
bool os_set_current_directory_path (const char* new_file_path);
char* os_get_file_part (const char* path);
bool os_check_file_exists (const char* path);
