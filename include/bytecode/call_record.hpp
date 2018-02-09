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
    Call_Parameter<T> result;
    uint8_t calling_convention;
    uint8_t param_count;

    void set_param (uint8_t index, Bytecode_Type bytecode_type, T* value) {
        this->parameters[index].bytecode_type = bytecode_type;
        memcpy(&this->parameters[index].value, value, sizeof(T));
    }

    void set_result (Bytecode_Type bytecode_type, T* value) {
        result.bytecode_type = bytecode_type;
        memcpy(&result.value, value, sizeof(T));
    }

    void reset () {
        memset(this->parameters, 0, sizeof(this->parameters));
        memset(&this->result, 0, sizeof(this->result));
        this->calling_convention = 0;
        this->param_count = 0;
    }
};
