#pragma once

#define PRINT_TABS for (size_t i = current_tabs; i > 0; i--) printf("    ");

struct Ast_Printer {
    size_t current_tabs;
    bool name_only = false;

    Ast_Printer (size_t default_tab_level = 0) { this->current_tabs = default_tab_level; }

    void print (Ast_Arguments* args) {
        printf("(");
        if (args->unnamed.size() > 0) {
            print(args->unnamed[0]);
            for (int i = 1; i < args->unnamed.size(); i++) {
                printf(", ");
                print(args->unnamed[i]);
            }
        }
        printf(")");
    }

    void print(Ast_Statement* stm) {
        this->name_only = false;
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
        printf("}");
    }

	void print(Ast_Declaration* decl) {
        printf("%s", decl->name);

        if (decl->type) {
            printf(" : ");
            print(decl->type);
        } else printf(" :");

        if (decl->expression) {
            if (decl->type) printf(" ");
            if (decl->is_constant) {
                printf(": ");
            } else printf("= ");
            print(decl->expression);
        }
	}

	void print(Ast_Return* ret) {
        printf("return");
        if (ret->expression) {
            printf(" ");
            print(ret->expression);
        }
	}

	void print(Ast_If* _if) {
        printf("if (");
        print(_if->condition);
        printf(") ");
		print(_if->then_body);
        if (_if->else_body) {
            printf(" else ");
            print(_if->else_body);
        }
	}

	void print(Ast_While* _while) {
        printf("while (");
        print(_while->condition);
        printf(") ");
		print(_while->body);
	}

	void print(Ast_Import* import) {
        printf("import \"%s\"", import->path);
    }

	void print(Ast_Foreign* foreign) {
        printf("foreign ");
        if (foreign->module_name) printf("%s ", foreign->module_name);
        if (foreign->function_name) printf("%s ", foreign->function_name);
        printf("{\n");
        for (auto decl : foreign->declarations) {
            print(decl + 1);
        }
        printf("}");
    }

	void print(Ast_Break*) {
        printf("break");
    }

	void print(Ast_Expression* exp) {
        if (exp->exp_type != AST_EXPRESSION_FUNCTION
                && exp->exp_type != AST_EXPRESSION_TYPE) {
            this->name_only = true;
        }
		switch (exp->exp_type) {
            case AST_EXPRESSION_FUNCTION: {
                print(reinterpret_cast<Ast_Function*>(exp));
                break;
            }
			case AST_EXPRESSION_RUN: {
				print(reinterpret_cast<Ast_Run*>(exp));
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
			case AST_EXPRESSION_TYPE: {
				print(reinterpret_cast<Ast_Type*>(exp));
				break;
			}
		}
	}

    void print(Ast_Run* run) {
        printf("#(");
        print(run->expression);
        printf(")");
    }

    void print(Ast_Function* func) {
        if (this->name_only) {
            printf(func->name);
        } else {
            print(func->type);
            printf(" ");
            print(func->body);
        }
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
                printf(" = ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_ATTRIBUTE: {
                printf(".");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_SUBSCRIPT: {
                printf("[");
                print(binary->rhs);
                printf("]");
                break;
            }
        	case AST_BINARY_LOGICAL_AND: {
                printf(" && ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LOGICAL_OR: {
                printf(" || ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_ADD: {
                printf(" + ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_SUB: {
                printf(" - ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_MUL: {
                printf(" * ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_DIV: {
                printf(" / ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_REM: {
                printf(" %% ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_AND: {
                printf(" & ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_OR: {
                printf(" | ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_XOR: {
                printf(" ^ ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_RIGHT_SHIFT: {
                printf(" >> ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_BITWISE_LEFT_SHIFT: {
                printf(" << ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_EQ: {
                printf(" == ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_NEQ: {
                printf(" != ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LT: {
                printf(" < ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_LTE: {
                printf(" <= ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_GT: {
                printf(" > ");
                print(binary->rhs);
                break;
            }
        	case AST_BINARY_GTE: {
                printf(" >= ");
                print(binary->rhs);
                break;
            }
        }
    }

    void print(Ast_Cast* cast) {
        printf("cast(");
        print(cast->cast_to);
        printf(") ");
        print(cast->value);
    }

    void print(Ast_Ident* ident) {
        printf("(");
        printf(ident->name);
        if (!ident->declaration) printf("?");
        printf(")");
    }

    void print(Ast_Literal* lit) {
        switch (lit->literal_type) {
            case AST_LITERAL_SIGNED_INT: {
                printf("%lld", lit->int_value);
                break;
            }
        	case AST_LITERAL_UNSIGNED_INT: {
                printf("%llu", lit->uint_value);
                break;
            }
        	case AST_LITERAL_DECIMAL: {
                printf("%lf", lit->decimal_value);
                break;
            }
        	case AST_LITERAL_STRING: {
                printf(lit->string_value);
                break;
            }
        }
    }

    void print(Ast_Type* type) {
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
        printf("fn");
        if (func_type->arg_decls.size() > 0) {
            printf(" (");
            print(func_type->arg_decls[0]);
            for (int i = 1; i < func_type->arg_decls.size(); i++) {
                printf(", ");
                print(func_type->arg_decls[i]);
            }
            printf(")");
        }
        if (func_type->ret_type) {
            printf(" -> ");
            print(func_type->ret_type);
        }
    }

    void print(Ast_Struct_Type* struct_type) {
        printf("{%s}", struct_type->name);
    }

    void print(Ast_Pointer_Type* ptr_type) {
        printf("*");
        print(ptr_type->base);
    }

    void print(Ast_Array_Type* array_type) {
        printf("[");
        print(array_type->length);
        printf("] ");
        print(array_type->base);
    }
};
