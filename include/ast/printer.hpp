#pragma once

#define PRINT_TABS for (size_t i = current_tabs; i > 0; i--) printf("    ");

struct Ast_Printer {
    size_t current_tabs;

    Ast_Printer (size_t default_tab_level = 0) { this->current_tabs = default_tab_level; }

    void print (Ast_Arguments* args) {
        printf("(");
        if (args->unnamed.size() > 0) {
            print(args->unnamed[0]);
            for (size_t i = 1; i < args->unnamed.size(); i++) {
                printf(", ");
                print(args->unnamed[i]);
            }
        }
        if (args->named.size() > 0) {
            if (args->unnamed.size()) printf(", ");

            auto it = args->named.begin();
            printf("%s = ", it->first);
            print(it->second);
            for (size_t i = 1; i < args->named.size(); i++) {
                it++;
                printf(", %s = ", it->first);
                print(it->second);
            }
        }
        printf(")");
    }

    void print(Ast_Statement* stm) {
        switch (stm->stm_type) {
			case AST_STATEMENT_SCOPE: {
				print(reinterpret_cast<Ast_Scope*>(stm));
				break;
			}
			case AST_STATEMENT_ASSIGN: {
				print(reinterpret_cast<Ast_Assign*>(stm));
				break;
			}
			case AST_STATEMENT_DEFER: {
				print(reinterpret_cast<Ast_Defer*>(stm));
				break;
			}
			case AST_STATEMENT_IF: {
				print(reinterpret_cast<Ast_If*>(stm));
				break;
			}
			case AST_STATEMENT_STATIC_IF: {
				print(reinterpret_cast<Ast_Static_If*>(stm));
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
			case AST_STATEMENT_FOREIGN: {
				print(reinterpret_cast<Ast_Foreign*>(stm));
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				print(reinterpret_cast<Ast_Expression*>(stm));
				break;
			}
			default: {
                printf("--UNDEFINED--");
                break;
            }
		}
        printf("\n");
    }

    void print(Ast_Scope* scope) {
        printf("{\n");
        this->current_tabs += 1;
        for (auto stm : scope->statements) {
            PRINT_TABS;
            print(stm);
        }
        this->current_tabs -= 1;
        PRINT_TABS;
        printf("}");
    }

    void print(Ast_Assign* assign) {
        print(assign->variable);
        printf(" = ");
        print(assign->value);
    }

    void print(Ast_Defer* defer) {
        printf("defer ");
        print(defer->statement);
    }

	void print(Ast_Declaration* decl, bool short_version = false) {
        printf(decl->names[0]);
        for (size_t i = 1; i < decl->names.size; i++) {
            printf(", %s", decl->names[i]);
        }

        if (decl->type) {
            printf(" : ");
            print(decl->type);
        } else printf(" :");

        if (decl->value) {
            if (decl->type) printf(" ");
            if (decl->is_constant) {
                printf(": ");
            } else printf("= ");

            print(decl->value, short_version);
        }
	}

	void print(Ast_Return* ret) {
        printf("return");
        if (ret->expression) {
            printf(" ");
            print(ret->expression);
        }
	}

	void print(Ast_Static_If* static_if) {
        printf("#");
        this->print(static_if->stm_if);
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
        if (import->is_include) {
            printf("include \"%s\"", import->path);
        } else {
            printf("import \"%s\"", import->path);
        }
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

	void print(Ast_Expression* exp, bool short_version = true) {
		switch (exp->exp_type) {
            case AST_EXPRESSION_FUNCTION: {
                print(reinterpret_cast<Ast_Function*>(exp), short_version);
                break;
            }
            case AST_EXPRESSION_COMMA_SEPARATED: {
                print(reinterpret_cast<Ast_Comma_Separated*>(exp), short_version);
                break;
            }
			case AST_EXPRESSION_IMPORT: {
				print(reinterpret_cast<Ast_Import*>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE: {
				print(reinterpret_cast<Ast_Type*>(exp), short_version);
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
			default: {
                printf("--UNDEFINED--");
                break;
            }
		}
	}

    void print(Ast_Run* run) {
        printf("#(");
        print(run->expression);
        printf(")");
    }

    void print(Ast_Function* func, bool short_version = true) {
        if (short_version) {
            printf("<%s>", func->name);
        } else {
            printf("fn");
            if (func->arg_scope->statements.size() > 0) {
                printf(" (");
                auto stm = func->arg_scope->statements[0];
                assert(stm->stm_type == AST_STATEMENT_DECLARATION);
                print(static_cast<Ast_Declaration*>(stm), true);
                for (int i = 1; i < func->arg_scope->statements.size(); i++) {
                    printf(", ");
                    stm = func->arg_scope->statements[i];
                    assert(stm->stm_type == AST_STATEMENT_DECLARATION);
                    print(static_cast<Ast_Declaration*>(stm), true);
                }
                printf(")");
            }

            if (func->ret_scope->statements.size() > 0) {
                printf(" -> (");
                auto stm = func->ret_scope->statements[0];
                assert(stm->stm_type == AST_STATEMENT_DECLARATION);
                auto decl = static_cast<Ast_Declaration*>(stm);
                if (decl->names.size > 0) print(decl, true);
                else print(decl->type, true);
                for (int i = 1; i < func->ret_scope->statements.size(); i++) {
                    printf(", ");
                    stm = func->ret_scope->statements[i];
                    assert(stm->stm_type == AST_STATEMENT_DECLARATION);
                    decl = static_cast<Ast_Declaration*>(stm);
                    if (decl->names.size > 0) print(decl, true);
                    else print(decl->type, true);
                }
                printf(")");
            }

            printf(" ");
            print(func->body);
        }
    }

    void print(Ast_Comma_Separated* comma_separated, bool short_version = true) {
        if (comma_separated->expressions.size > 0) {
            auto exp = comma_separated->expressions[0];
            print(exp, short_version);
            for (size_t i = 1; i < comma_separated->expressions.size; i++) {
                printf(", ");
                exp = comma_separated->expressions[i];
                print(exp, short_version);
            }
        }
    }

    void print(Ast_Function_Call* call) {
        print(call->func);
        print(call->arguments);
    }

    void print(Ast_Unary* unary) {
        switch (unary->unary_op) {
            case AST_UNARY_DEREFERENCE: {
                printf("&(");
                print(unary->exp);
                printf(")");
                break;
            }
        	case AST_UNARY_REFERENCE: {
                printf("*(");
                print(unary->exp);
                printf(")");
                break;
            }
        	case AST_UNARY_NOT: {
                printf("!(");
                print(unary->exp);
                printf(")");
                break;
            }
        	case AST_UNARY_NEGATE: {
                printf("-(");
                print(unary->exp);
                printf(")");
                break;
            }
        }
    }

    void print(Ast_Binary* binary) {
        print(binary->lhs);
        switch (binary->binary_op) {
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
        if (ident->declaration) {
            printf("%s", ident->name);
        } else {
            printf("(%s)", ident->name);
        }
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
                printf("\"%s\"", lit->string_value);
                break;
            }
        	case AST_LITERAL_BOOL: {
                if (lit->bool_value) {
                    printf("true");
                } else printf("false");
                break;
            }
        }
    }

    void print(Ast_Type* type, bool short_version = true) {
        switch (type->typedef_type) {
        	case AST_TYPEDEF_FUNCTION : {
                print(reinterpret_cast<Ast_Function_Type*>(type));
                break;
            }
        	case AST_TYPEDEF_STRUCT : {
                print(reinterpret_cast<Ast_Struct_Type*>(type), short_version);
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
        	case AST_TYPEDEF_SLICE : {
                print(reinterpret_cast<Ast_Slice_Type*>(type));
                break;
            }
        }
    }

    void print(Ast_Function_Type* func_type) {
        printf("{fn");
        if (func_type->arg_types.size() > 0) {
            printf(" (");
            print(func_type->arg_types[0]);
            for (int i = 1; i < func_type->arg_types.size(); i++) {
                printf(", ");
                print(func_type->arg_types[i]);
            }
            printf(")");
        }
        if (func_type->ret_types.size() > 0) {
            printf(" -> (");
            print(func_type->ret_types[0]);
            for (int i = 1; i < func_type->ret_types.size(); i++) {
                printf(", ");
                print(func_type->ret_types[i]);
            }
            printf(")");
        }
        printf("}");
    }

    void print(Ast_Struct_Type* struct_type, bool short_version = true) {
        if (short_version) {
            printf("{%s}", struct_type->name);
        } else {
            printf("struct ");
            this->print(&struct_type->scope);
        }
    }

    void print(Ast_Pointer_Type* ptr_type) {
        printf("*");
        print(ptr_type->base);
    }

    void print(Ast_Array_Type* array_type) {
        printf("[");
        if (array_type->length) {
            print(array_type->length);
        }
        printf("]");
        print(array_type->base);
    }

    void print(Ast_Slice_Type* slice_type) {
        printf("[]");
        print(slice_type->get_typed_base());
    }
};
