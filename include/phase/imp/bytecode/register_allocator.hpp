#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"

#include "compiler_events.hpp"
#include "ast/ast_factory.hpp"

#include "bytecode/instructions.hpp"
#include "bytecode/interpreter.hpp"
#include "bytecode/constants.hpp"
#include "bytecode/globals.hpp"

#include "util/logger.hpp"

#include <vector>
#include <assert.h>

struct Register_State {
    Ast_Declaration* decl = NULL;
    bool expired = false;

    Register_State () {}
    Register_State (Ast_Declaration* decl) { this->decl = decl; }
};

struct Register_Allocator : Phase, Ast_Navigator {
    Interpreter* interpreter;

    std::vector<Register_State*> decl_regs;

    size_t registers_used = 0;
    bool is_left_value = false;
    size_t max_registers;

    Register_Allocator (Interpreter* interpreter)
            : Phase("Register Allocator", CE_BYTECODE_ALLOCATE_REGISTERS) {
        this->interpreter = interpreter;
        // @TODO this value could change? maybe the interpreter should have
        // "infinite" registers, and the assembly code generator should
        // handle the actual phisical number of registers (spilling)
        this->max_registers = 16;
    }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

    void allocate_function (Ast_Function* func) {
        auto tmp = this->decl_regs;
        this->decl_regs.clear();

        for (auto decl : func->type->arg_decls) {
            decl_regs.push_back(new Register_State(decl));
        }

        if (func->scope) Ast_Navigator::ast_handle(func->scope);

        this->decl_regs = tmp;
    }

    void ast_handle (Ast_Statement* stm) {
        for (auto reg_state : this->decl_regs) {
            if (!reg_state->decl) reg_state->expired = true;
        }
        Ast_Navigator::ast_handle(stm);
    }

    void ast_handle (Ast_Declaration* decl) {
        if (decl->is_constant) {
			if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
	            auto func = static_cast<Ast_Function*>(decl->expression);
	            if (!func->foreign_module_name) {
	    			this->allocate_function(func);
	            }
			}
	    } else {
            Ast_Navigator::ast_handle(decl);

            if (decl->expression) {
                if (decl->expression->inferred_type->is_primitive) {
                    this->set_declaration(decl);
                } else decl->is_spilled = true;
            } else decl->is_spilled = true;
		}
    }

	void ast_handle (Ast_Expression* exp) {
		if (this->is_left_value != false) {
			auto tmp = this->is_left_value;
			this->is_left_value = false;
			Ast_Navigator::ast_handle(exp);
			this->is_left_value = tmp;
		} else Ast_Navigator::ast_handle(exp);
	}

	void ast_handle_left (Ast_Expression* exp) {
		if (this->is_left_value != true) {
			auto tmp = this->is_left_value;
			this->is_left_value = true;
			Ast_Navigator::ast_handle(exp);
			this->is_left_value = tmp;
		} else Ast_Navigator::ast_handle(exp);
	}

    void ast_handle (Ast_Function* func) {
        func->reg = reserve_next_reg();
    }

    void ast_handle (Ast_Literal* lit) {
        lit->reg = reserve_next_reg();
    }

    void ast_handle (Ast_Ident* ident) {
        auto decl_index = get_decl_index(ident->declaration);
        if (decl_index < 0) {
            ident->reg = reserve_next_reg();
        } else {
            ident->reg = (uint8_t) decl_index;
        }
    }

    void ast_handle (Ast_Run* run) {
        Ast_Navigator::ast_handle(run);

        run->reg = run->expression->reg;
    }

    void ast_handle (Ast_Binary* binop) {
        switch (binop->binary_op) {
			case AST_BINARY_ATTRIBUTE: {
                this->ast_handle_left(binop->lhs);
                Ast_Navigator::ast_handle(binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
			case AST_BINARY_SUBSCRIPT: {
                this->ast_handle_left(binop->lhs);
                Ast_Navigator::ast_handle(binop->rhs);

                binop->reg = reserve_next_reg();
				break;
			}
			case AST_BINARY_ASSIGN: {
                this->ast_handle_left(binop->lhs);
                Ast_Navigator::ast_handle(binop->rhs);

				break;
			}
			default: {
				Ast_Navigator::ast_handle(binop->lhs);
				Ast_Navigator::ast_handle(binop->rhs);

                binop->reg = reserve_reg(binop->lhs->reg, binop->rhs->reg);
				break;
			}
		}
    }

    void ast_handle (Ast_Unary* unop) {
		switch (unop->unary_op) {
			case AST_UNARY_NOT: {
				Ast_Navigator::ast_handle(unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);
				break;
			}
			case AST_UNARY_NEGATE: {
				Ast_Navigator::ast_handle(unop->exp);
                unop->reg = reserve_reg(unop->exp->reg);
				break;
			}
			case AST_UNARY_REFERENCE: {
				this->ast_handle_left(unop->exp);
                unop->reg = unop->exp->reg;

                if (unop->exp->exp_type == AST_EXPRESSION_IDENT) {
                    auto ident = static_cast<Ast_Ident*>(unop->exp);
                    ident->declaration->is_spilled = true;
                }

				break;
			}
	        case AST_UNARY_DEREFERENCE: {
				this->ast_handle(unop->exp);
                if (!this->is_left_value) {
                    unop->reg = reserve_reg(unop->exp->reg);
                } else unop->reg = unop->exp->reg;
	            break;
	        }
			default: abort();
		}
    }

	void ast_handle (Ast_Cast* cast) {
		Ast_Navigator::ast_handle(cast->value);

        cast->reg = reserve_reg(cast->value->reg);
	}

    void ast_handle (Ast_Function_Call* call) {
        call->reg = reserve_next_reg();

        if (call->func->exp_type != AST_EXPRESSION_FUNCTION) {
            Ast_Navigator::ast_handle(call->func);
        }

        Ast_Navigator::ast_handle(call->arguments);
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

	void print_pipe_metrics () {
		print_extra_metric("Registers used:", "%zd", this->registers_used);
	}
};
