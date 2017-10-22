#pragma once

#include <stdint.h>

const uint8_t BYTECODE_STOP = 0x01;
const uint8_t BYTECODE_NOOP = 0x02;

const uint8_t BYTECODE_ALLOCA = 0x03;
const uint8_t BYTECODE_LOAD   = 0x04;
const uint8_t BYTECODE_STORE  = 0x05;

const uint8_t BYTECODE_ADD = 0x06;
const uint8_t BYTECODE_SUB = 0x07;
const uint8_t BYTECODE_MUL = 0x08;
const uint8_t BYTECODE_DIV = 0x09;
