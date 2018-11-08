#include "report.hpp"

#include "compiler.hpp"
#include "parser/ast.hpp"

#define VA_REPORT(name, buffer)													\
	va_list argptr;																\
	va_start(argptr, format);													\
	_report(name, buffer, location, format, argptr);							\
	va_end(argptr)

void print_location (FILE* buffer, Location* location) {
	if (location) {
		if (location->filename) {
			fprintf(buffer, "\t@ %s:%zd,%zd\n", location->filename, location->line, location->col);
		} else {
			fprintf(buffer, "\t@ [INTERNAL]\n");
		}
	}
}

void _report (const char* level, FILE* buffer, Location* location, char* format, va_list argptr) {
	fprintf(buffer, "\n[%s] ", level);
    vfprintf(buffer, format, argptr);
	fprintf(buffer, "\n");
	print_location(buffer, location);
}

void report_debug (Location* location, char* format, ...) {
	VA_REPORT("DEBUG", stdout);
}

void report_info (Location* location, char* format, ...) {
	VA_REPORT("INFO", stdout);
}

void report_warning (Location* location, char* format, ...) {
	VA_REPORT("WARNING", stdout);
}

void report_error (Location* location, char* format, ...) {
	VA_REPORT("ERROR", stderr);
}

void report_internal (Location* location, char* format, ...) {
	VA_REPORT("INTERNAL ERROR", stderr);
}

void report_error_and_stop (Location* location, char* format, ...) {
	VA_REPORT("ERROR", stderr);
	Compiler::instance->quit();
}
