#include "report.hpp"

#include "compiler.hpp"
#include "parser/ast.hpp"

void print_location (FILE* buffer, Location* location) {
	fprintf(buffer, "\t@ %s:%zd,%zd\n", location->filename, location->line, location->col);
}

void _report (const char* level, FILE* buffer, Location* location, char* format, va_list argptr) {
	fprintf(buffer, "\n[%s] ", level);
    vfprintf(buffer, format, argptr);
	fprintf(buffer, "\n");
	if (location) print_location(buffer, location);
}

void report_debug (Location* location, char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	_report("DEBUG", stdout, location, format, argptr);
    va_end(argptr);
}

void report_info (Location* location, char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	_report("INFO", stdout, location, format, argptr);
    va_end(argptr);
}

void report_warning (Location* location, char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	_report("WARNING", stdout, location, format, argptr);
    va_end(argptr);
}

void report_error (Location* location, char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	_report("ERROR", stderr, location, format, argptr);
    va_end(argptr);
}

void report_error_stop (Location* location, char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	_report("ERROR", stderr, location, format, argptr);
    va_end(argptr);
	g_compiler->stop();
}
