#include "compiler.hpp"

#include "platform.hpp"

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\" (Ignored)"

#define COMPILER_DONE_FORMAT "\nCompleted in %8.6fs (%8.6fs)\n"

#define CHECK_ARG(arg) (strcmp(argv[i], arg) == 0)
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

Compiler* Compiler::instance = NULL;

Compiler_Settings::Compiler_Settings (int argc, char** argv) {
	this->handle_arguments(argc, argv);
	os_get_current_directory(this->initial_path);
}

void Compiler_Settings::handle_arguments (int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (CHECK_ARG_2("-o", "-output")) {
				this->output = argv[++i];
			} else if (CHECK_ARG_2("-v", "-verbose")) {
				this->is_verbose = true;
			} else if (CHECK_ARG_2("-d", "-debug")) {
				this->is_debug = true;
			} else report_warning(NULL, UKNOWN_ARG_FORMAT, i, argv[i]);
		} else {
			auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
			os_get_absolute_path(argv[i], absolute_path);
			this->input_files.push_back(absolute_path);
		}
	}
}

Compiler::Compiler (Compiler_Settings* settings) {
	this->settings = settings;
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

	printf("\n");
	this->modules->parser->print_metrics(userInterval);
	this->modules->pipeline->print_metrics(userInterval);

    printf(COMPILER_DONE_FORMAT, userInterval, wallInterval);
}

void Compiler::quit () {
	exit(1);
}

Compiler* Compiler::create(int argc, char** argv) {
	return Compiler::create(new Compiler_Settings(argc, argv));
}

Compiler* Compiler::create(Compiler_Settings* settings) {
	Compiler::instance = new Compiler(settings);
	return Compiler::instance;
}

void Compiler::destroy() {
	delete Compiler::instance;
	Compiler::instance = NULL;
}
