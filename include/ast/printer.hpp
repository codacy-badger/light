#pragma once

#include <cstdarg>

#define PRINT_TABS for (size_t i = current_tabs; i > 0; i--) printf("    ");

struct Ast_Printer {
    size_t current_tabs;

    Ast_Printer (size_t default_tab_level = 0) { this->current_tabs = default_tab_level; }

    void print (Ast_Arguments* args) {
        print("(");
        if (args->unnamed.size() > 0) {
            print(args->unnamed[0]);
            for (int i = 1; i < args->unnamed.size(); i++) {
                print(", ");
                print(args->unnamed[i]);
            }
        }
        print(")");
    }

    void print(Ast_Statement* stm) {
        switch (stm->stm_type) {
			case AST_STATEMENT_SCOPE: {
				print(reinterpret_cast<Ast_Scope*>(stm));
				break;
			}
			case AST_STATEMENT_IF: {
				print(reinterpret_cast<Ast_If*>(stm));
				break;
			}
			case AST_STATEMENT_WHILE: {
				print(reinterpret_cast<Ast_While*>(stm));
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				print(reinterpret_cast<Ast_Declaration*>(stm));
				break;
			}
			case AST_STATEMENT_RETURN: {
				print(reinterpret_cast<Ast_Return*>(stm));
				break;
			}
			case AST_STATEMENT_BREAK: {
				print(reinterpret_cast<Ast_Break*>(stm));
				break;
			}
			case AST_STATEMENT_IMPORT: {
				print(reinterpret_cast<Ast_Import*>(stm));
				break;
			}
			case AST_STATEMENT_FOREIGN: {
				print(reinterpret_cast<Ast_Foreign*>(stm));
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				print(reinterpret_cast<Ast_Expression*>(stm));
				break;
			}
			default: break;
		}
    }

    void print(Ast_Scope* scope) {
        printf("{\n");
        this->current_tabs += 1;
        for (auto stm : scope->statements) {
            PRINT_TABS;
            print(stm);
            printf("\n");
        }
        this->current_tabs -= 1;
        PRINT_TABS;
        printf("}\n");
    }

	void print(Ast_Declaration* decl) {
        printf("%s ", decl->name);

        if (decl->type) {
            print(": ");
            print(decl->type);
        } else print(":");

        if (decl->expression) {
            if (decl->is_constant) {
                print(": ");
            } else print("= ");
            print(decl->expression);
        }
	}

	void print(Ast_Return* ret) {
        print("return");
        if (ret->expression) {
            print(" ");
            print(ret->expression);
        }
	}

	void print(Ast_If* _if) {
        print("if (");
        print(_if->condition);
        print(") ");
		print(_if->then_scope);
        if (_if->else_scope) {
            print(" else ");
            print(_if->else_scope);
        }
	}

	void print(Ast_While* _while) {
        print("while (");
        print(_while->condition);
        print(") ");
		print(_while->scope);
	}

	void print(Ast_Import* import) {
        print("import");
        if (import->include) print("!");
        print(" \"%s\"", import->path);
    }

	void print(Ast_Foreign* foreign) {
        print("foreign ");
        if (foreign->module_name) print("%s ", foreign->module_name);
        if (foreign->function_name) print("%s ", foreign->function_name);
        print("{\n");
        for (auto decl : foreign->declarations) {
            print(decl + 1);
        }
        print("}");
    }

	void print(Ast_Break*) {
        print("break");
    }

	void print(Ast_Expression* exp) {
		switch (exp->exp_type) {
			case AST_EXPRESSION_RUN: {
				print(reinterpret_cast<Ast_Run*>(exp));
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				print(reinterpret_cast<Ast_Function*>(exp));
				break;
			}
			case AST_EXPRESSION_CALL: {
				print(reinterpret_cast<Ast_Function_Call*>(exp));
				break;
			}
			case AST_EXPRESSION_BINARY: {
				print(reinterpret_cast<Ast_Binary*>(exp));
				break;
			}
			case AST_EXPRESSION_UNARY: {
				print(reinterpret_cast<Ast_Unary*>(exp));
				break;
			}
			case AST_EXPRESSION_CAST: {
				print(reinterpret_cast<Ast_Cast*>(exp));
				break;
			}
			case AST_EXPRESSION_IDENT: {
				print(reinterpret_cast<Ast_Ident*>(exp));
				break;
			}
			case AST_EXPRESSION_LITERAL: {
				print(reinterpret_cast<Ast_Literal*>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE_INSTANCE: {
				print(reinterpret_cast<Ast_Type_Instance*>(exp));
				break;
			}
		}
	}

    void print(Ast_Run* run) {
        print("#(");
        print(run->expression);
        print(")");
    }

    void print(Ast_Function* func) {
        print(func->type);
        print(" ");
        print(func->scope);
    }

    void print(Ast_Function_Call* call) {
        print(call->func);
        print(call->arguments);
    }

    void print(Ast_Unary* unary) {
        switch (unary->unary_op) {
            case AST_UNARY_DEREFERENCE: {
                printf("&");
                break;
            }
        	case AST_UNARY_REFERENCE: {
                printf("*");
                break;
            }
        	case AST_UNARY_NOT: {
                printf("!");
                break;
            }
        	case AST_UNARY_NEGATE: {
                printf("-");
                break;
            }
        }
        print(unary->exp);
    }

    void print(Ast_Binary* binary) {
        print(binary->lhs);
        switch (binary->binary_op) {
            case AST_BINARY_ASSIGN: {
                print(" = ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_ATTRIBUTE: {
                print(".");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_SUBSCRIPT: {
                print("[");
                print(binary->rhs);
                print("]");
                break;
            }
        	case AST_BINARY_LOGICAL_AND: {
                print(" && ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LOGICAL_OR: {
                print(" || ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_ADD: {
                print(" + ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_SUB: {
                print(" - ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_MUL: {
                print(" * ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_DIV: {
                print(" / ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_REM: {
                print(" %% ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_AND: {
                print(" & ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_OR: {
                print(" | ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_XOR: {
                print(" ^ ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_RIGHT_SHIFT: {
                print(" >> ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_LEFT_SHIFT: {
                print(" << ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_EQ: {
                print(" == ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_NEQ: {
                print(" != ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LT: {
                print(" < ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LTE: {
                print(" <= ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_GT: {
                print(" > ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_GTE: {
                print(" >= ");
                print(binary->rhs);
                break;
            }
        }
    }

    void print(Ast_Cast* cast) {
        print("cast(");
        print(cast->cast_to);
        print(") ");
        print(cast->value);
    }

    void print(Ast_Ident* ident) {
        print(ident->name);
    }

    void print(Ast_Literal* lit) {
        switch (lit->literal_type) {
            case AST_LITERAL_SIGNED_INT: {
                print("%lld", lit->int_value);
                break;
            }
        	case AST_LITERAL_UNSIGNED_INT: {
                print("%llu", lit->uint_value);
                break;
            }
        	case AST_LITERAL_DECIMAL: {
                print("%lf", lit->int_value);
                break;
            }
        	case AST_LITERAL_STRING: {
                print(lit->string_value);
                break;
            }
        }
    }

    void print(Ast_Type_Instance* type) {
        switch (type->typedef_type) {
        	case AST_TYPEDEF_FUNCTION : {
                print(reinterpret_cast<Ast_Function_Type*>(type));
                break;
            }
        	case AST_TYPEDEF_STRUCT : {
                print(reinterpret_cast<Ast_Struct_Type*>(type));
                break;
            }
        	case AST_TYPEDEF_POINTER : {
                print(reinterpret_cast<Ast_Pointer_Type*>(type));
                break;
            }
        	case AST_TYPEDEF_ARRAY : {
                print(reinterpret_cast<Ast_Array_Type*>(type));
                break;
            }
        }
    }

    void print(Ast_Function_Type* func_type) {
        print("fn");
        if (func_type->arg_decls.size() > 0) {
            print(" (");
            print(func_type->arg_decls[0]);
            for (int i = 1; i < func_type->arg_decls.size(); i++) {
                print(", ");
                print(func_type->arg_decls[i]);
            }
            print(")");
        }
        if (func_type->ret_type) {
            print(" -> ");
            print(func_type->ret_type);
        }
    }

    void print(Ast_Struct_Type* struct_type) {
        print(struct_type->name);
    }

    void print(Ast_Pointer_Type* ptr_type) {
        print("*");
        print(ptr_type->base);
    }

    void print(Ast_Array_Type* array_type) {
        print("[");
        print(array_type->length);
        print("] ");
        print(array_type->base);
    }

    void print(const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
    }
};
