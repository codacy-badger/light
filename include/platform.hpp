#pragma once

#include <stdio.h>
#include <stdint.h>

uint64_t os_get_time ();
double os_clock_stop (uint64_t start);

void* os_get_module (const char* module_name);
void* os_get_function (void* module, const char* function_name);

void os_get_current_directory (char* buffer);
void os_get_absolute_path (const char* relative_path, char* buffer, char** file_part);
bool os_set_current_directory (char* new_path);
