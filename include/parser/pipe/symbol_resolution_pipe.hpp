#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <map>

struct cmp_str {
    bool operator()(const char *a, const char *b) const {
        return strcmp(a, b) < 0;
    }
};

struct Symbol_Resolution : Pipe {
    std::map<const char*, Ast_Expression*, cmp_str> cache;

    void onStatement(Ast_Statement* stm);
	void onFinish ();
};
