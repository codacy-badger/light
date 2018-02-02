#pragma once

#include "bytecode/instructions.hpp"

#define MAX_CALL_PARAMETERS 16

template<typename T>
struct Call_Parameter {
    Bytecode_Type bytecode_type;
    T value;
};

template<typename T>
struct Call_Record {
    Call_Parameter<T> parameters[MAX_CALL_PARAMETERS];
    uint8_t calling_convention;
    uint8_t param_count;

    void set_param (uint8_t index, Bytecode_Type bytecode_type, T* value) {
        this->parameters[index].bytecode_type = bytecode_type;
        memcpy(&this->parameters[index].value, value, sizeof(T));
    }

    void reset (uint8_t cc, uint8_t count) {
        memset(this->parameters, 0, sizeof(this->parameters));
        this->calling_convention = cc;
        this->param_count = count;
    }
};
