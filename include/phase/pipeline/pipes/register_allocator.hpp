#pragma once

#include "phase/pipeline/pipe.hpp"

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

    size_t registers_used = 0;
    bool is_left_value = false;

    size_t max_registers;

	Register_Allocator (size_t max_registers) {
        this->pipe_name = "Register_Allocator";
        this->max_registers = max_registers;
    }

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

        if (decl->is_constant) {
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

	void handle (Ast_Expression** exp_ptr) {
		if (this->is_left_value != false) {
			auto tmp = this->is_left_value;
			this->is_left_value = false;
			Pipe::handle(exp_ptr);
			this->is_left_value = tmp;
		} else Pipe::handle(exp_ptr);
	}

	void handle_left (Ast_Expression** exp_ptr) {
		if (this->is_left_value != true) {
			auto tmp = this->is_left_value;
			this->is_left_value = true;
			Pipe::handle(exp_ptr);
			this->is_left_value = tmp;
		} else Pipe::handle(exp_ptr);
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

    void handle (Ast_Directive_Run** run_ptr) {
        Pipe::handle(run_ptr);

        (*run_ptr)->reg = (*run_ptr)->expression->reg;
    }

    void handle (Ast_Binary** binop_ptr) {
        auto binop = (*binop_ptr);

        switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
                this->handle_left(&binop->lhs);
                Pipe::handle(&binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
			case AST_BINARY_SUBSCRIPT: {
                this->handle_left(&binop->lhs);
                Pipe::handle(&binop->rhs);

                binop->reg = reserve_next_reg();
				break;
			}
			case AST_BINARY_ASSIGN: {
                this->handle_left(&binop->lhs);
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
				this->handle_left(&unop->exp);
                unop->reg = unop->exp->reg;

                if (unop->exp->exp_type == AST_EXPRESSION_IDENT) {
                    auto ident = static_cast<Ast_Ident*>(unop->exp);
                    ident->declaration->is_spilled = true;
                }

				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				this->handle(&unop->exp);
                if (!this->is_left_value) {
                    unop->reg = reserve_reg(unop->exp->reg);
                } else unop->reg = unop->exp->reg;
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

        Pipe::handle(&call->arguments);
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

        auto reg_size = decl_regs.size();
        if (this->registers_used < reg_size) {
            this->registers_used = reg_size;
        }

        return (uint8_t) (reg_size - 1);
    }

    void set_declaration (Ast_Declaration* decl) {
        ASSERT(decl->expression->reg >= 0);
        ASSERT(this->decl_regs[decl->expression->reg]);
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

	void print_pipe_metrics () {
		PRINT_METRIC("Registers used:        %zd / %zd",
            this->registers_used, this->max_registers);
	}
};