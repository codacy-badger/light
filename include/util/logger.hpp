#pragma once

#include "ast/ast.hpp"
#include "util/location.hpp"
#include "util/events.hpp"
#include "compiler_events.hpp"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

enum Log_Level : uint8_t {
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INTERNAL,
};

#define LEVEL_ALIAS(name, level) static void                                    \
    name (const char* format, ...) {                                            \
    va_list argptr; va_start(argptr, format);                                   \
    auto output = Logger::log(level, format, argptr);                           \
    fprintf(output, "\n\n");                                                    \
    va_end(argptr); }

#define LOCATION_LEVEL_ALIAS(name, level) static void                           \
    name (Location* location, const char* format, ...) {                        \
    va_list argptr; va_start(argptr, format);                                   \
    auto output = Logger::log(level, location, format, argptr);                 \
    fprintf(output, "\n\n");                                                    \
    va_end(argptr); }

#define AST_NODE_LEVEL_ALIAS(name, level) static void                           \
    name (Ast* node, const char* format, ...) {                                 \
    va_list argptr; va_start(argptr, format);                                   \
    Logger::log(level, node, format, argptr);                                   \
    va_end(argptr); }

struct Logger {
    static FILE* log (Log_Level level, const char* format, va_list argptr) {
        auto output = Logger::get_output_buffer(level);

        fprintf(output, "[%s] ", Logger::get_level_string(level));
        vfprintf(output, format, argptr);

        return output;
    }

    static FILE* log (Log_Level level, Location* location, const char* format, va_list argptr) {
        auto output = Logger::log(level, format, argptr);
    	fprintf(output, "\n");

        if (location) {
    		if (location->filename) {
    			fprintf(output, "\t@ %s:%zd", location->filename, location->line);
    		} else {
    			fprintf(output, "\t@ [Compiler defined]");
    		}
    	}

        return output;
    }

    static FILE* log (Log_Level level, Ast* node, const char* format, va_list argptr) {
        if (node != NULL) {
            return Logger::log(level, &node->location, format, argptr);
        } else {
            auto output = Logger::log(level, format, argptr);
            fprintf(output, "\n\n");
            return output;
        }
    }

    LEVEL_ALIAS(verbose,    LOG_LEVEL_VERBOSE)
    LEVEL_ALIAS(info,       LOG_LEVEL_INFO)
    LEVEL_ALIAS(debug,      LOG_LEVEL_DEBUG)
    LEVEL_ALIAS(warning,    LOG_LEVEL_WARNING)
    LEVEL_ALIAS(error,      LOG_LEVEL_ERROR)
    LEVEL_ALIAS(internal,   LOG_LEVEL_INTERNAL)

    LOCATION_LEVEL_ALIAS(verbose,   LOG_LEVEL_VERBOSE)
    LOCATION_LEVEL_ALIAS(info,      LOG_LEVEL_INFO)
    LOCATION_LEVEL_ALIAS(debug,     LOG_LEVEL_DEBUG)
    LOCATION_LEVEL_ALIAS(warning,   LOG_LEVEL_WARNING)
    LOCATION_LEVEL_ALIAS(error,     LOG_LEVEL_ERROR)
    LOCATION_LEVEL_ALIAS(internal,  LOG_LEVEL_INTERNAL)

    AST_NODE_LEVEL_ALIAS(verbose,   LOG_LEVEL_VERBOSE)
    AST_NODE_LEVEL_ALIAS(info,      LOG_LEVEL_INFO)
    AST_NODE_LEVEL_ALIAS(debug,     LOG_LEVEL_DEBUG)
    AST_NODE_LEVEL_ALIAS(warning,   LOG_LEVEL_WARNING)
    AST_NODE_LEVEL_ALIAS(error,     LOG_LEVEL_ERROR)
    AST_NODE_LEVEL_ALIAS(internal,  LOG_LEVEL_INTERNAL)

    static void error_and_stop (const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        auto output = Logger::log(LOG_LEVEL_ERROR, format, argptr);
        fprintf(output, "\n\n");
        va_end(argptr);
    }

    static void error_and_stop (Location* location, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        auto output = Logger::log(LOG_LEVEL_ERROR, location, format, argptr);
        fprintf(output, "\n\n");
        va_end(argptr);
    }

    static void error_and_stop (Ast* node, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        auto output = Logger::log(LOG_LEVEL_ERROR, node, format, argptr);
        fprintf(output, "\n\n");
        va_end(argptr);
    }

    static const char* get_level_string (Log_Level level) {
        switch (level) {
            case LOG_LEVEL_VERBOSE:     return "VERBOSE";
            case LOG_LEVEL_INFO:        return "INFO";
            case LOG_LEVEL_DEBUG:       return "DEBUG";
            case LOG_LEVEL_WARNING:     return "WARNING";
            case LOG_LEVEL_ERROR:       return "ERROR";
            case LOG_LEVEL_INTERNAL:    return "INTERNAL";
            default:                    return "?";
        }
    }

    static FILE* get_output_buffer (Log_Level level) {
        switch (level) {
            case LOG_LEVEL_ERROR:
            case LOG_LEVEL_INTERNAL:    return stderr;
            default:                    return stdout;
        }
    }

    static void stop_compilation () {
        Events::trigger(CE_COMPILER_ERROR);
    }
};

#ifndef CUSTOM_DEBUG
#define assert(condition) /* empty */
#else
#define assert(condition)   __pragma(warning(push))                             \
                            __pragma(warning(disable:4127))					    \
                            if (!(condition)) { Logger::error(		            \
                                "Assertion failed: %s\n\t@ %s, line %d",        \
                                #condition, __FILE__, __LINE__); }              \
                            __pragma(warning(pop))
#endif
