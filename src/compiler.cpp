#pragma once

#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/print_pipe.hpp"

Ast_Type_Instance* Light_Compiler::type_def_void 	= new Ast_Struct_Type("void");
Ast_Type_Instance* Light_Compiler::type_def_i1 		= new Ast_Struct_Type("i1");
Ast_Type_Instance* Light_Compiler::type_def_i32 	= new Ast_Struct_Type("i32");

void link (std::string output) {
	auto linker = Timer::getTime();
	std::string linkerCommand = "link /nologo /ENTRY:main ";

	linkerCommand += "/OUT:\"" + output + "\" ";

	linkerCommand += "_tmp_.obj ";
	linkerCommand += "build\\std_li.lib";

	system(linkerCommand.c_str());
	system("del _tmp_.obj");
	Timer::print("  Link  ", linker);
}

Light_Compiler::Light_Compiler (Light_Compiler_Settings* settings) {
	if (!settings)
		this->settings = new Light_Compiler_Settings();
	else this->settings = settings;
}

void Light_Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		printf("%s\n", filename);

		auto parser = new Parser(filename);
		parser->append(new Symbol_Resolution());
		parser->append(new PrintPipe());
		//parser->append(new LLVMPipe("_tmp_.obj"));

		auto start = Timer::getTime();
		parser->block();
		Timer::print("  Parse ", start);
		parser->onFinish();
	}
	Timer::print("TOTAL   ", total);
}

void print_node_location (FILE* buffer, Ast* node) {
	assert(node);
	fprintf(buffer, "%s:%d,%d", node->filename, node->line, node->col);
}

void Light_Compiler::report_info (Ast* node, char* format, ...) {
	fprintf(stderr, "[INFO] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
}

void Light_Compiler::report_warning (Ast* node, char* format, ...) {
	fprintf(stderr, "[WARNING] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
}

void Light_Compiler::report_error (Ast* node, char* format, ...) {
	fprintf(stderr, "[ERROR] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	fprintf(stderr, "\n\t");

	print_node_location(stderr, node);
}
