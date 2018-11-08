#pragma once

#include <stdio.h>
#include <stdarg.h>

struct Location {
	const char* filename = NULL;
	size_t line = 1, col = 1;
};

void print_location (FILE* buffer, Location* location);

void report_debug 		(Location* location, char* format, ...);
void report_info 		(Location* location, char* format, ...);
void report_warning 	(Location* location, char* format, ...);
void report_error 		(Location* location, char* format, ...);
void report_internal	(Location* location, char* format, ...);
void report_error_and_stop 	(Location* location, char* format, ...);

#define WARN(node, ...) report_warning(&node->location, __VA_ARGS__)
#define ERROR(node, ...) report_error(&node->location, __VA_ARGS__)
#define ERROR_STOP(node, ...) report_error_and_stop(&node->location, __VA_ARGS__)
