#pragma once

#include <stdio.h>
#include <stdint.h>

typedef uint64_t Timer_Function();

uint64_t os_get_wall_time ();
uint64_t os_get_user_time ();
double os_time_wall_stop (uint64_t start);
double os_time_user_stop (uint64_t start);

void* os_get_module (const char* module_name);
void* os_get_function (void* module, const char* function_name);

void os_get_current_directory (char* buffer);
void os_get_absolute_path (const char* relative_path, char* buffer, char** file_part);
bool os_set_current_directory (char* new_path);
