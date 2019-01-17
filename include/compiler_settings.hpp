#pragma once

#include "util/os.hpp"
#include "util/arch.hpp"

#include <vector>

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\""
#define UKNOWN_LOGGER_LEVEL "Unkown logger level at %d: \"%s\""
#define UKNOWN_LOGGER_LEVEL_TIP "Use one of the following: VERBOSE, INFO, DEBUG, WARNING or ERROR"

#define CHECK_ARG(arg) (strcmp(argv[i], arg) == 0)
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

#define WARN(msg) Logger::warning(msg, i, argv[i])
#define WARN_TIP(msg, tip) WARN(msg "\n\t" tip)

struct Compiler_Settings {
	std::vector<char*> input_files;
	const char* output = NULL;

	char initial_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_verbose = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	Compiler_Settings () {
		os_get_current_directory(this->initial_path);
	}

    void handle_arguments (int argc, char** argv) {
    	for (int i = 1; i < argc; i++) {
    		if (argv[i][0] == '-') {
    			if (CHECK_ARG_2("-o", "-output")) {
    				this->output = argv[++i];
    			} else if (CHECK_ARG_2("-v", "-verbose")) {
    				this->is_verbose = true;
    			} else if (CHECK_ARG_2("-d", "-debug")) {
    				this->is_debug = true;
    			} else if (CHECK_ARG_2("-mt", "-multithread")) {
    				this->is_multithread = true;
    			} else if (CHECK_ARG_2("-l", "-log")) {
					auto requested_log_level = Logger::get_level_by_string(argv[++i]);
					if (requested_log_level == LOG_LEVEL_UNDEFINED) {
						WARN_TIP(UKNOWN_LOGGER_LEVEL, UKNOWN_LOGGER_LEVEL_TIP);
					} else Logger::current_level = requested_log_level;
    			} else WARN(UKNOWN_ARG_FORMAT);
    		} else {
    			auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
    			os_get_absolute_path(argv[i], absolute_path);
    			this->input_files.push_back(absolute_path);
    		}
    	}
    }
};
