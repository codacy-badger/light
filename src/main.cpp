#include "workspace.hpp"
#include "platform.hpp"

#define COMPILER_NAME "Light"
#define COMPILER_V_MAJOR 0
#define COMPILER_V_MINOR 0
#define COMPILER_V_PATCH 0

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define COMPILER_VERSION  STR(COMPILER_V_MAJOR) "." STR(COMPILER_V_MINOR) "." STR(COMPILER_V_PATCH)

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\""

#define CHECK_ARG(arg) (str_eq(argv[i], arg, sizeof(arg)))
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

#define WARN(msg) printf(msg, i, argv[i])

void apply_build_settings (Build_Context* context, int argc, char** argv);
bool str_eq (const char* str1, size_t len1, const char* str2, size_t len2);
bool str_eq (const char* str1, const char* str2, size_t len2);

int main (int argc, char** argv) {
    printf(COMPILER_NAME " v" COMPILER_VERSION "\n\n");
    auto start = os_get_time();

    auto workspace = Workspace::create("default");
    apply_build_settings(&workspace->context, argc, argv);

    workspace->start_building();
    workspace->wait_for_end();
    workspace->stop_building();

    printf("\nDone in %8.6f\n", os_time_stop(start));
}

void apply_build_settings (Build_Context* context, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (CHECK_ARG_2("-o", "-output")) {
                context->output = argv[++i];
            } else if (CHECK_ARG_2("-v", "-version")) {
                printf(COMPILER_NAME " v" COMPILER_VERSION "\n");
                exit(0);
            } else if (CHECK_ARG_2("-d", "-debug")) {
                context->is_debug = true;
            } else if (CHECK_ARG_2("-mt", "-multithread")) {
                context->is_multithread = true;
            } else WARN(UKNOWN_ARG_FORMAT);
        } else {
            context->input_files.push_back(argv[i]);
        }
    }
}

bool str_eq (const char* str1, const char* str2, size_t len2) {
    return str_eq(str1, strlen(str1), str2, len2);
}

bool str_eq (const char* str1, size_t len1, const char* str2, size_t len2) {
    if (len1 != len2) return false;

    for (size_t i = 0; i < len1; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }

    return true;
}
