#include "compiler.hpp"

#include "platform.hpp"

#define COMPILER_DONE_FORMAT "\nCompleted in %8.6fs (%8.6fs)\n"

Compiler* Compiler::instance = NULL;

void Compiler_Settings::handle_arguments (int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (CHECK_ARG_2("-o", "-output")) {
				this->output_file = argv[++i];
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
	if (settings == NULL) {
		this->settings = new Compiler_Settings();
	} else this->settings = settings;

	os_get_current_directory(this->settings->initial_path);
}

void Compiler::run () {
	printf("%s v%s\n", LIGHT_NAME, LIGHT_VERSION);

    auto totalWall = os_get_wall_time();
    auto totalUser = os_get_user_time();
    for (auto filename : Compiler::instance->settings->input_files) {
		this->pipeline->run(filename);
    }
    printf(COMPILER_DONE_FORMAT, os_time_user_stop(totalUser), os_time_wall_stop(totalWall));
}

void Compiler::add_import (Ast_Import* import) {
	this->pipeline->pending_imports.push_back(import);
}

void Compiler::quit () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}

Compiler* Compiler::create(int argc, char** argv) {
	Compiler::instance = new Compiler();
	Compiler::instance->settings->handle_arguments(argc, argv);
	return Compiler::instance;
}

Compiler* Compiler::create(Compiler_Settings* settings) {
	Compiler::instance = new Compiler(settings);
	return Compiler::instance;
}

void Compiler::destroy() {
	delete Compiler::instance;
	Compiler::instance = NULL;
}
