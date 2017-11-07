#include "bytecode/bytecode_generator.hpp"

#include "cff.hpp"

#include <windows.h>

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm);
    this->to_next(stm);
}

void Bytecode_Generator::gen (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            this->gen(static_cast<Ast_Declaration*>(stm));
            break;
        }
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Declaration* decl) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
        this->gen(decl->expression);
    }
}

void Bytecode_Generator::gen (Ast_Expression* exp) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION: {
            this->gen(static_cast<Ast_Function*>(exp));
            break;
        }
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Function* fn) {
    printf("GEN function: %s\n", fn->name);

    if (fn->foreign_module_name) {
        HMODULE module = LoadLibrary(fn->foreign_module_name);
        cff_func_ret_t f1 = (cff_func_ret_t) GetProcAddress(module, fn->name);

        CFF_Argument* args[1];
        args[0] = new CFF_Argument((uint32_t)-11);

        uint32_t handle = cff_invoke(f1, args, 1);
        printf("%s(-11) -> %X\n", fn->name, handle);
    }
}
