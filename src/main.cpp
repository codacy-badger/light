#include "compiler.hpp"

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\""

#define CHECK_ARG(arg) (strcmp(argv[i], arg) == 0)
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

#define WARN(msg) printf(msg, i, argv[i])

void apply_build_settings (Build_Settings* settings, int argc, char** argv);

int main (int argc, char** argv) {
    auto workspace = Compiler::create_workspace();

    auto settings = new Build_Settings();
    apply_build_settings(settings, argc, argv);
    Compiler::apply_settings(workspace, settings);

    Compiler::begin_build(workspace);
    Compiler::wait_for_end(workspace);

    printf("\nCompilation done!");
}

void apply_build_settings (Build_Settings* settings, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (CHECK_ARG_2("-o", "-output")) {
                settings->output = argv[++i];
            } else if (CHECK_ARG_2("-v", "-version")) {
                printf(COMPILER_NAME " v" COMPILER_VERSION "\n");
                exit(0);
            } else if (CHECK_ARG_2("-d", "-debug")) {
                settings->is_debug = true;
            } else if (CHECK_ARG_2("-mt", "-multithread")) {
                settings->is_multithread = true;
            } else WARN(UKNOWN_ARG_FORMAT);
        } else {
            auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
            os_get_absolute_path(argv[i], absolute_path);
            settings->input_files.push_back(absolute_path);
        }
    }
}
