#pragma once

#include <cstdarg>

struct Ast_Printer {
    static void print(Ast_Scope* scope, uint8_t tabs = 0) {
        print(tabs, "{");
        for (auto stm : scope->statements) {
            print(stm, tabs + 1);
        }
        print(tabs, "}\n");
    }

    static void print(Ast_Statement* stm, uint8_t tabs = 0) {
        switch (stm->stm_type) {
			case AST_STATEMENT_SCOPE: {
				print(reinterpret_cast<Ast_Scope*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_IF: {
				print(reinterpret_cast<Ast_If*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_WHILE: {
				print(reinterpret_cast<Ast_While*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				print(reinterpret_cast<Ast_Declaration*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_RETURN: {
				print(reinterpret_cast<Ast_Return*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_BREAK: {
				print(reinterpret_cast<Ast_Break*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_IMPORT: {
				print(reinterpret_cast<Ast_Import*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_FOREIGN: {
				print(reinterpret_cast<Ast_Foreign*>(stm), tabs);
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
                print(tabs, "");
				print(reinterpret_cast<Ast_Expression*>(stm));
                print("\n");
				break;
			}
			default: break;
		}
    }

	static void print(Ast_Declaration* decl, uint8_t tabs) {
        print(tabs, "%s ", decl->name);

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

        print(";\n");
	}

	static void print(Ast_Return* ret, uint8_t tabs) {
        print(tabs, "return");
        if (ret->expression) {
            print(" ");
            print(ret->expression, tabs);
        }
        print(";\n");
	}

	static void print(Ast_If* _if, uint8_t tabs) {
        print(tabs, "if (");
        print(_if->condition);
        print(") ");
		print(_if->then_scope);
        if (_if->else_scope) {
            print(" else ");
            print(_if->else_scope);
        }
	}

	static void print(Ast_While* _while, uint8_t tabs) {
        print(tabs, "while (");
        print(_while->condition);
        print(") ");
		print(_while->scope);
	}

	static void print(Ast_Import* import, uint8_t tabs) {
        print(tabs, "import");
        if (import->include) print("!");
        print(" \"%s\"\n", import->path);
    }

	static void print(Ast_Foreign* foreign, uint8_t tabs) {
        print(tabs, "foreign ");
        if (foreign->module_name) print("%s ", foreign->module_name);
        if (foreign->function_name) print("%s ", foreign->function_name);
        print("{\n");
        for (auto decl : foreign->declarations) {
            print(decl, tabs + 1);
        }
        print(tabs, "}\n");
    }

	static void print(Ast_Break*, uint8_t) { /* empty */ }

	static void print(Ast_Expression*) {
        print("<EXP>");
		/*switch (exp->exp_type) {
			case AST_EXPRESSION_RUN: {
				this->ast_handle(reinterpret_cast<Ast_Run*>(exp));
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				this->ast_handle(reinterpret_cast<Ast_Function*>(exp));
				break;
			}
			case AST_EXPRESSION_CALL: {
				this->ast_handle(reinterpret_cast<Ast_Function_Call*>(exp));
				break;
			}
			case AST_EXPRESSION_BINARY: {
				this->ast_handle(reinterpret_cast<Ast_Binary*>(exp));
				break;
			}
			case AST_EXPRESSION_UNARY: {
				this->ast_handle(reinterpret_cast<Ast_Unary*>(exp));
				break;
			}
			case AST_EXPRESSION_CAST: {
				this->ast_handle(reinterpret_cast<Ast_Cast*>(exp));
				break;
			}
			case AST_EXPRESSION_IDENT: {
				this->ast_handle(reinterpret_cast<Ast_Ident*>(exp));
				break;
			}
			case AST_EXPRESSION_LITERAL: {
				this->ast_handle(reinterpret_cast<Ast_Literal*>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE_INSTANCE: {
				this->ast_handle(reinterpret_cast<Ast_Type_Instance*>(exp));
				break;
			}
		}*/
	}

    static void print(Ast_Arguments*, uint8_t) { /* empty */ }

    static void print(uint8_t tabs, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        while (tabs > 0) {
            printf("    ");
            tabs -= 1;
        }
        vprintf(format, argptr);
        va_end(argptr);
    }

    static void print(const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        vprintf(format, argptr);
        va_end(argptr);
    }
};
