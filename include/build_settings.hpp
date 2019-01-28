#pragma once

#include "utils/os.hpp"
#include "utils/arch.hpp"

#include <vector>

struct Build_Settings {
	const char* output = NULL;
	std::vector<const char*> input_files;

	char initial_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	Build_Settings () {
		os_get_current_directory(this->initial_path);
	}
};
