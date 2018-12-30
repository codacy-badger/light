#include "compiler.hpp"

#include "platform.hpp"

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

#define COMPILER_DONE_FORMAT "\nCompleted in %8.6fs (%8.6fs)\n"

Compiler* Compiler::inst = NULL;

Compiler::Compiler (int argc, char** argv) {
	if (argc > 0) {
		this->settings->handle_arguments(argc, argv);
	}
	Compiler::inst = this;
}

void Compiler::run () {
	printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

	for (auto filename : this->settings->input_files) {
		auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
	    os_get_absolute_path(filename, absolute_path);

		this->code_sources.push(absolute_path);
	}

    auto totalWall = os_get_wall_time();
    auto totalUser = os_get_user_time();

	while (!this->code_sources.empty()) {
		auto absolute_path = this->code_sources.front();
		this->code_sources.pop();

		this->modules->get_module(absolute_path);

		// TODO: send the main module to output
	}

	auto userInterval = os_time_user_stop(totalUser);
	auto wallInterval = os_time_wall_stop(totalWall);

	this->modules->print_metrics(userInterval);

    printf(COMPILER_DONE_FORMAT, userInterval, wallInterval);
}

void Compiler::quit () {
	exit(1);
}
