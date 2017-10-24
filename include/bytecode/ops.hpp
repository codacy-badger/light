#pragma once

#include <stdint.h>

const uint8_t BYTECODE_STOP = 0x01;
const uint8_t BYTECODE_NOOP = 0x02;

const uint8_t BYTECODE_SET  = 0x03;
const uint8_t BYTECODE_COPY = 0x04;

const uint8_t BYTECODE_STACK_ALLOCA = 0x05;
const uint8_t BYTECODE_STACK_OFFSET = 0x06;

const uint8_t BYTECODE_LOAD 	  = 0x07;
const uint8_t BYTECODE_STORE 	  = 0x08;
const uint8_t BYTECODE_STORE_INT  = 0x09;
const uint8_t BYTECODE_STORE_REAL = 0x09;

const uint8_t BYTECODE_ADD 		= 0x21;
const uint8_t BYTECODE_ADD_INT 	=  0x22;
