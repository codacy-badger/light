#pragma once

#include "steps/async_pipe.hpp"
#include "front/parser/parser.hpp"

struct Parse_Step : Async_Pipe {
    Parser* parser = NULL;

    Parse_Step () : Async_Pipe("Parser") { /* empty */ }

    void setup () {
        auto c = this->context;
        auto internal_scope = new Internal_Scope(c->target_arch, c->target_os);
        this->parser = new Parser(internal_scope);
    }

    void handle (void* in) {
        auto absolute_path = reinterpret_cast<const char*>(in);

        size_t length = 0;
        auto text = read_full_source(absolute_path, &length);
        auto global_scope = this->parser->build_ast(text, length, absolute_path);

        for (auto stm : global_scope->statements) {
            this->pipe_out((void*) stm);
        }

        delete text;
    }

    const char* read_full_source (const char* absolute_path, size_t* length_ptr) {
        FILE* file = NULL;

        auto error_code = fopen_s(&file, absolute_path, "r");
        if (error_code != 0) {
			char buffer[256];
			strerror_s(buffer, sizeof buffer, error_code);
			printf("Cannot open file '%s': %s", absolute_path, buffer);
		}

        fseek(file, 0L, SEEK_END);
        auto length = ftell(file);
        rewind(file);

        // @TODO @Incomplete check if the calloc call suceeded
        auto text = (char*) malloc(length);
        memset(text, 0, length);

        // @TODO @Incomplete check if we need to make the buffer bigger
        (*length_ptr) = fread((void*) text, 1, length, file);
        while (!feof(file)) {
            (*length_ptr) += fread((void*) (text + (*length_ptr)), 1, length, file);
        }

		fclose(file);
        return text;
    }
};
