#pragma once

#include "parser/pipes.hpp"

#include <vector>

using namespace std;

struct Register_State {
    Ast_Declaration* decl = NULL;
    bool expired = false;

    Register_State () {}
    Register_State (Ast_Declaration* decl) { this->decl = decl; }
};

struct Register_Allocator : Pipe {
    vector<Register_State*> decl_regs;

    void allocate_function (Ast_Function* func) {
        auto tmp = this->decl_regs;
        this->decl_regs.clear();

        for (auto decl : func->arg_decls) {
            decl_regs.push_back(new Register_State(decl));
        }

        if (func->scope) Pipe::handle(&func->scope);

        this->decl_regs = tmp;
    }

    void handle (Ast_Statement** stm_ptr) {
        for (auto reg_state : this->decl_regs) {
            if (!reg_state->decl) reg_state->expired = true;
        }
        Pipe::handle(stm_ptr);
    }

    void handle (Ast_Declaration** decl_ptr) {
        auto decl = (*decl_ptr);

        if (decl->is_constant()) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
	            auto func = static_cast<Ast_Function*>(decl->expression);
	            if (!func->foreign_module_name) {
	    			this->allocate_function(func);
	            }
			}
	    } else {
            Pipe::handle(decl_ptr);
			this->decl_regs[decl->expression->reg] = new Register_State(decl);
		}
    }

    void handle (Ast_Function** func_ptr) {
        auto func = (*func_ptr);

        func->reg = (uint8_t) this->decl_regs.size();
        this->decl_regs.push_back(new Register_State());
    }

    void handle (Ast_Literal** lit_ptr) {
        auto lit = (*lit_ptr);

        lit->reg = (uint8_t) this->decl_regs.size();
        this->decl_regs.push_back(new Register_State());
    }

    void handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        auto decl_index = get_decl_index(ident->declaration);
        if (decl_index < 0) {
            ident->reg = reserve_next_reg();
        } else {
            ident->reg = (uint8_t) decl_index;
            ident->is_in_register = true;
        }
    }

    void handle (Ast_Binary** binop_ptr) {
        auto binop = (*binop_ptr);

        switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
                Pipe::handle(&binop->lhs);
                Pipe::handle(&binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
			case AST_BINARY_SUBSCRIPT: {
                Pipe::handle(&binop->lhs);
                Pipe::handle(&binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
			case AST_BINARY_ASSIGN: {
                Pipe::handle(&binop->lhs);
                Pipe::handle(&binop->rhs);
				break;
			}
			default: {
				Pipe::handle(&binop->lhs);
				Pipe::handle(&binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
		}
    }

    void handle (Ast_Function_Call** call_ptr) {
        auto call = (*call_ptr);

        call->reg = reserve_next_reg();

        //Pipe::handle(&call->fn);
		for (auto &exp : call->arguments) {
			Pipe::handle(&exp);
		}
    }

    int16_t get_decl_index (Ast_Declaration* decl) {
        for (uint8_t i = 0; i < this->decl_regs.size(); i++) {
            if (this->decl_regs[i]->decl == decl) return i;
        }
        return -1;
    }

    uint8_t reserve_next_reg () {
        for (uint8_t i = 0; i < this->decl_regs.size(); i++) {
            if (this->decl_regs[i]->expired) {
                this->decl_regs[i]->expired = false;
                return i;
            }
        }
        decl_regs.push_back(new Register_State());
        return (uint8_t) decl_regs.size();
    }

    bool has_declaration (uint8_t index) {
        return this->decl_regs[index]->decl;
    }

    uint8_t reserve_reg (uint8_t reg1, uint8_t reg2) {
        if (!has_declaration(reg1)) return reg1;
        if (!has_declaration(reg2)) return reg2;
        return reserve_next_reg();
    }
};
