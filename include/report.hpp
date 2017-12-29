#pragma once

#include <stdio.h>
#include <stdarg.h>

struct Ast;

struct Location {
	const char* filename = NULL;
	size_t line = 0, col = 0;
};

void print_location (FILE* buffer, Location* location);

void report_debug 		(Location* location, char* format, ...);
void report_info 		(Location* location, char* format, ...);
void report_warning 	(Location* location, char* format, ...);
void report_error 		(Location* location, char* format, ...);
void report_error_stop 	(Location* location, char* format, ...);
