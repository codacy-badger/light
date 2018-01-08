#pragma once

#include <stdio.h>
#include <stdint.h>

uint64_t os_get_time ();
double os_clock_stop (uint64_t start);

void* os_get_module (const char* module_name);
void* os_get_function (void* module, const char* function_name);
