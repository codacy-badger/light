#pragma once

#include <stdio.h>
#include <stdarg.h>

const uint16_t COFF_MACHINE_UNKNOWN = 0x0;
const uint16_t COFF_MACHINE_AMD64 = 0x8664;

struct COFF_Header {
	uint16_t machine;
	uint16_t numberOfSections;
	uint32_t timeDateStamp;
	uint32_t pointerToSymbolTable;
	uint32_t numberOfSymbols;
	uint16_t sizeOfOptionalHeader;
	uint16_t characteristics;
};

const uint32_t COFF_SECTION_CNT_CODE = 0x00000020;
const uint32_t COFF_SECTION_CNT_INITIALIZED_DATA = 0x00000040;
const uint32_t COFF_SECTION_CNT_UNINITIALIZED_DATA = 0x00000080;

const uint32_t COFF_SECTION_MEM_DISCARDABLE = 0x02000000;
const uint32_t COFF_SECTION_MEM_SHARED = 0x10000000;
const uint32_t COFF_SECTION_MEM_EXECUTE = 0x20000000;
const uint32_t COFF_SECTION_MEM_READ = 0x40000000;
const uint32_t COFF_SECTION_MEM_WRITE = 0x80000000;

const uint32_t COFF_SECTION_ALIGN_1BYTES = 0x00100000;
const uint32_t COFF_SECTION_ALIGN_2BYTES = 0x00200000;
const uint32_t COFF_SECTION_ALIGN_4BYTES = 0x00300000;
const uint32_t COFF_SECTION_ALIGN_8BYTES = 0x00400000;
const uint32_t COFF_SECTION_ALIGN_16BYTES = 0x00500000;
const uint32_t COFF_SECTION_ALIGN_32BYTES = 0x00600000;
const uint32_t COFF_SECTION_ALIGN_64BYTES = 0x00700000;
const uint32_t COFF_SECTION_ALIGN_128BYTES = 0x00800000;
const uint32_t COFF_SECTION_ALIGN_256BYTES = 0x00900000;
const uint32_t COFF_SECTION_ALIGN_512BYTES = 0x00A00000;
const uint32_t COFF_SECTION_ALIGN_1024BYTES = 0x00B00000;
const uint32_t COFF_SECTION_ALIGN_2048BYTES = 0x00C00000;
const uint32_t COFF_SECTION_ALIGN_4096BYTES = 0x00D00000;
const uint32_t COFF_SECTION_ALIGN_8192BYTES = 0x00E00000;

struct COFF_Section_Header {
	char		name[8];
	uint32_t	virtualSize;
	uint32_t	virtualAddress;
	uint32_t	sizeOfRawData;
	uint32_t	pointerToRawData;
	uint32_t	pointerToRawRelocations;
	uint32_t	pointerToLineNumbers;
	uint16_t	numberOfRelocations;
	uint16_t	numberOfLineNumbers;
	uint32_t	characteristics;
};

void write_1b (FILE* file, int number) {
	fputc(number, file);
}

void write_data (FILE* file, size_t count, char* data) {
	for(int i = 0; i < count; i++)
		fputc(*(data + i), file);
}

void write_1b_var (FILE* file, size_t count, ...) {
	va_list argptr;
    va_start(argptr, count);
	for(size_t i = 0; i < count; i++)
		fputc(va_arg(argptr, char), file);
    va_end(argptr);
}
