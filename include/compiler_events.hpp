#pragma once

#include <stdint.h>
#include "ast/ast.hpp"

enum Event_Kind : uint8_t {
    EVENT_UNDEFINED = 0,

    EVENT_FILE,
    EVENT_PHASE,
    EVENT_COMPLETE,
};

struct Compiler_Event {
    Event_Kind kind = EVENT_UNDEFINED;
};

struct Compiler_Event_File {
    bool is_open;
    const char* filename;
    const char* absolute_path;
};

enum Phase_Kind : uint8_t {
    PHASE_UNDEFINED = 0,

    PHASE_TYPE_CHECK,
    PHASE_TARGET_CODE,
    PHASE_EXECUTABLE,
};

struct Compiler_Event_Phase {
    Phase_Kind phase_kind = PHASE_UNDEFINED;
    Ast_Declaration* declaration;
};
