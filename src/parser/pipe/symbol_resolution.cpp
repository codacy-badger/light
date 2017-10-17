#pragma once

#include "parser/pipe/symbol_resolution.hpp"

#include "compiler.hpp"

Symbol_Resolution::Symbol_Resolution () {
    this->cache["void"] =   Light_Compiler::type_def_void;
    this->cache["i1"]   =   Light_Compiler::type_def_i1;
    this->cache["i32"]  =   Light_Compiler::type_def_i32;
}

void Symbol_Resolution::onDeclaration (Ast_Declaration* decl) {
	if (decl->type) {
        /*if (decl->type->type_inst_type == AST_TYPE_INST_STRUCT) {
            auto structDecl = static_cast<Ast_Struct_Type*>(decl->type);
            auto it = this->cache.find(structDecl->name);
            if (it != this->cache.end()) printf("Found! -> %s\n", it->first);
        }*/
		this->toNext(decl);
	} else {
        printf("Declaration type is not specified!\n");
    }
}
