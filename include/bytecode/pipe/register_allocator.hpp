#pragma once

#include "parser/pipes.hpp"

#include <assert.h>
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

	PIPE_NAME(Register_Allocator)

    void allocate_function (Ast_Function* func) {
        auto tmp = this->decl_regs;
        this->decl_regs.clear();

        for (auto decl : func->type->arg_decls) {
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
            if (decl->expression) {
                if (decl->expression->inferred_type->is_primitive) {
                    this->set_declaration(decl);
                } else decl->is_spilled = true;
            } else decl->is_spilled = true;
		}
    }

    void handle (Ast_Function** func_ptr) {
        auto func = (*func_ptr);
        func->reg = reserve_next_reg();
    }

    void handle (Ast_Literal** lit_ptr) {
        auto lit = (*lit_ptr);
        lit->reg = reserve_next_reg();
    }

    void handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        auto decl_index = get_decl_index(ident->declaration);
        if (decl_index < 0) {
            ident->reg = reserve_next_reg();
        } else {
            ident->reg = (uint8_t) decl_index;
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

    void handle (Ast_Unary** unop_ptr) {
        auto unop = (*unop_ptr);

		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
				Pipe::handle(&unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);
				break;
			}
			case AST_UNARY_NEGATE: {
				Pipe::handle(&unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);
				break;
			}
			case AST_UNARY_REFERENCE: {
				Pipe::handle(&unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);

                if (unop->exp->exp_type == AST_EXPRESSION_IDENT) {
                    auto ident = static_cast<Ast_Ident*>(unop->exp);
                    ident->declaration->is_spilled = true;
                }

				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				Pipe::handle(&unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);
	            break;
	        }
			default: abort();
		}
    }

	void handle (Ast_Cast** cast_ptr) {
		auto cast = (*cast_ptr);

		Pipe::handle(&cast->value);

        cast->reg = reserve_reg(cast->value->reg);
	}

    void handle (Ast_Function_Call** call_ptr) {
        auto call = (*call_ptr);

        call->reg = reserve_next_reg();

        if (call->func->exp_type != AST_EXPRESSION_FUNCTION) {
            Pipe::handle(&call->func);
        }

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
        return (uint8_t) (decl_regs.size() - 1);
    }

    void set_declaration (Ast_Declaration* decl) {
        assert(decl->expression->reg >= 0);
        assert(this->decl_regs[decl->expression->reg]);
        this->decl_regs[decl->expression->reg]->decl = decl;
    }

    bool has_declaration (uint8_t index) {
        return this->decl_regs[index]->decl;
    }

    uint8_t reserve_reg (uint8_t reg) {
        if (!has_declaration(reg)) return reg;
        return reserve_next_reg();
    }

    uint8_t reserve_reg (uint8_t reg1, uint8_t reg2) {
        if (!has_declaration(reg1)) return reg1;
        if (!has_declaration(reg2)) return reg2;
        return reserve_next_reg();
    }
};
