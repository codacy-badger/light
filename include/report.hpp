#pragma once

#include <stdio.h>
#include <stdarg.h>

struct Location {
	const char* filename = NULL;
	size_t line = 1;
};

void print_location (FILE* buffer, Location* location);

void report_debug 			(Location* location, char* format, ...);
void report_info 			(Location* location, char* format, ...);
void report_warning 		(Location* location, char* format, ...);
void report_error 			(Location* location, char* format, ...);
void report_internal		(Location* location, char* format, ...);
void report_error_and_stop 	(Location* location, char* format, ...);

#define DEBUG(node, ...) 		report_debug(&node->location, __VA_ARGS__)
#define INFO(node, ...) 		report_info(&node->location, __VA_ARGS__)
#define WARN(node, ...) 		report_warning(&node->location, __VA_ARGS__)
#define ERROR(node, ...) 		report_error(&node->location, __VA_ARGS__)
#define INTERNAL(node, ...) 	report_internal(&node->location, __VA_ARGS__)
#define ERROR_STOP(node, ...) 	report_error_and_stop(&node->location, __VA_ARGS__)

#ifndef CUSTOM_DEBUG
#define ASSERT(condition) /* empty */
#else
#define ASSERT(condition) 		__pragma(warning(push))							\
								__pragma(warning(disable:4127))					\
								if (!(condition)) { report_internal(NULL,		\
									"Assertion failed: %s\n\t@ %s, line %d",	\
									#condition, __FILE__, __LINE__); }			\
								__pragma(warning(pop))
#endif
