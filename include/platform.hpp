#pragma once

#include <stdint.h>

#define MAX_PATH_LENGTH 260

typedef uint64_t Timer_Function();

uint64_t os_get_wall_time ();
uint64_t os_get_user_time ();
double os_time_wall_stop (uint64_t start);
double os_time_user_stop (uint64_t start);

void* os_get_module (const char* module_name);
void* os_get_function (void* module, const char* function_name);

void os_get_current_directory (char* buffer);
void os_get_absolute_path (const char* relative_path, char* buffer);
bool os_set_current_directory (const char* new_path);
bool os_set_current_directory_path (const char* new_file_path);
char* os_get_file_part (const char* path);
