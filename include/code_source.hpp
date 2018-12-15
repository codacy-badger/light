#pragma once

enum Code_Source_Type {
    CODE_SOURCE_UNDEFINED,
    CODE_SOURCE_FILE,
    CODE_SOURCE_STRING,
};

struct Code_Source {
    Code_Source_Type type = CODE_SOURCE_UNDEFINED;
};

struct File_Code_Source : Code_Source {
    char* absolute_path;

    File_Code_Source(char* absolute_path) {
        this->type = CODE_SOURCE_FILE;
        this->absolute_path = absolute_path;
    }
};

struct String_Code_Source : Code_Source {
    char* source;

    String_Code_Source(char* source) {
        this->type = CODE_SOURCE_STRING;
        this->source = source;
    }
};