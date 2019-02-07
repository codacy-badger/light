#include "workspace.hpp"
#include "platform.hpp"

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\""

#define CHECK_ARG(arg) (str_eq(argv[i], arg, sizeof(arg)))
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

#define WARN(msg) printf(msg, i, argv[i])

void apply_build_settings (Build_Context* context, int argc, char** argv);
bool str_eq (const char* str1, size_t len1, const char* str2, size_t len2);
bool str_eq (const char* str1, const char* str2, size_t len2);

int main (int argc, char** argv) {
    auto start = os_get_time();

    auto workspace = Workspace::create_workspace();
    apply_build_settings(&workspace->context, argc, argv);

    workspace->wait_for_full_build();

    if (workspace->has_error) {
        printf("\nErrors found, stopping compilation...\n");
    } else {
        printf("\nDone in %8.6f\n", os_time_stop(start));
    }
}

void apply_build_settings (Build_Context* context, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (CHECK_ARG_2("-o", "-output")) {
                context->output = argv[++i];
            } else if (CHECK_ARG_2("-v", "-version")) {
                printf("DUH\n");
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
