#pragma once

#include "ast/ast.hpp"
#include "util/location.hpp"
#include "util/events.hpp"
#include "compiler_events.hpp"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <mutex>

enum Log_Level : uint8_t {
    LOG_LEVEL_UNDEFINED = 0,

    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INTERNAL,

    LOG_LEVEL_NONE,
};

#define LEVEL_ALIAS(name, level) static void                                    \
    name (const char* format, ...) {                                            \
    va_list argptr; va_start(argptr, format);                                   \
    Logger::log(level, format, argptr);                                         \
    va_end(argptr); }

#define LOCATION_LEVEL_ALIAS(name, level) static void                           \
    name (Location* location, const char* format, ...) {                        \
    va_list argptr; va_start(argptr, format);                                   \
    Logger::log(level, location, format, argptr);                               \
    va_end(argptr); }

#define AST_NODE_LEVEL_ALIAS(name, level) static void                           \
    name (Ast* node, const char* format, ...) {                                 \
    va_list argptr; va_start(argptr, format);                                   \
    Logger::log(level, node, format, argptr);                                   \
    va_end(argptr); }

struct Logger {
    static Log_Level current_level;

    static void log (Log_Level level, const char* format, va_list argptr) {
        Logger::log(level, (Location*)NULL, format, argptr);
    }

    static void log (Log_Level level, Ast* node, const char* format, va_list argptr) {
        if (node != NULL) {
            Logger::log(level, &node->location, format, argptr);
        } else {
            Logger::log(level, format, argptr);
        }
    }

    static void log (Log_Level level, Location* location, const char* format, va_list argptr) {
        if (Logger::should_print(level)) {
            auto output = Logger::get_output_buffer(level);

            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            fprintf(output, "[%s] ", Logger::get_level_string(level));
            vfprintf(output, format, argptr);
        	fprintf(output, "\n");

            if (location) {
        		if (location->filename) {
        			fprintf(output, "\t@ %s", location->filename);
                    if (location->line > 0) {
            			fprintf(output, ":%zd\n", location->line);
            		} else fprintf(output, "\n");
        		} else fprintf(output, "\t@ UNDEFINED\n");
        	}
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
        Logger::log(LOG_LEVEL_ERROR, format, argptr);
        va_end(argptr);
        Logger::stop_compilation();
    }

    static void error_and_stop (Location* location, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        Logger::log(LOG_LEVEL_ERROR, location, format, argptr);
        va_end(argptr);
        Logger::stop_compilation();
    }

    static void error_and_stop (Ast* node, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        Logger::log(LOG_LEVEL_ERROR, node, format, argptr);
        va_end(argptr);
        Logger::stop_compilation();
    }

    static bool should_print (Log_Level level) {
        return Logger::current_level <= level;
    }

    static bool is_debug () { return Logger::should_print(LOG_LEVEL_DEBUG); }
    static bool is_verbose () { return Logger::should_print(LOG_LEVEL_VERBOSE); }

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

    static Log_Level get_level_by_string (const char* level_string) {
        if (strcmp(level_string, "VERBOSE") == 0)   return LOG_LEVEL_VERBOSE;
        if (strcmp(level_string, "INFO") == 0)      return LOG_LEVEL_INFO;
        if (strcmp(level_string, "DEBUG") == 0)     return LOG_LEVEL_DEBUG;
        if (strcmp(level_string, "WARNING") == 0)   return LOG_LEVEL_WARNING;
        if (strcmp(level_string, "ERROR") == 0)     return LOG_LEVEL_ERROR;
        if (strcmp(level_string, "INTERNAL") == 0)  return LOG_LEVEL_INTERNAL;
        return LOG_LEVEL_UNDEFINED;
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
