#pragma once

#include <stdint.h>

struct Workspace;
struct Ast_Statement;

enum Event_Kind : uint8_t {
    EVENT_UNDEFINED = 0,

    EVENT_FILE,
    EVENT_PHASE,

    EVENT_COMPLETE,
};

struct Compiler_Event {
    Event_Kind kind = EVENT_UNDEFINED;

    Compiler_Event () { /* empty */ }
    Compiler_Event (Event_Kind kind) { this->kind = kind; }
};

enum File_Event_Kind : uint8_t {
    FILE_UNDEFINED = 0,

    FILE_OPEN,
    FILE_CLOSE,
};

struct Compiler_Event_File : Compiler_Event {
    File_Event_Kind file_kind = FILE_UNDEFINED;
    const char* absolute_path = NULL;
    const char* name = NULL;

    Compiler_Event_File (File_Event_Kind file_kind) : Compiler_Event(EVENT_FILE) {
        this->file_kind = file_kind;
    }
};

enum Phase_Kind : uint8_t {
    PHASE_UNDEFINED = 0,

    PHASE_PARSED,
    PHASE_TYPE_INFERRED,
    PHASE_TYPE_CHECK,
    PHASE_TARGET_CODE,
    PHASE_EXECUTABLE,
};

struct Compiler_Event_Phase : Compiler_Event {
    Phase_Kind phase_kind = PHASE_UNDEFINED;
    Ast_Statement* statement = NULL;

    Compiler_Event_Phase () : Compiler_Event(EVENT_PHASE) {}
    Compiler_Event_Phase (Ast_Statement* stm, Phase_Kind pk) : Compiler_Event(EVENT_PHASE) {
        this->statement = stm;
        this->phase_kind = pk;
    }
};
