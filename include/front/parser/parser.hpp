#pragma once

#include "lexer/lexer.hpp"
#include "ast/nodes.hpp"
#include "ast/types.hpp"
#include "ast/ast_factory.hpp"
#include "internal_scope.hpp"

#include <vector>

#define DEFAULT_FILE_EXTENSION ".li"

#define AST_NEW(T, ...) Ast_Factory::create<T>(&this->lexer, __VA_ARGS__)

struct Parser {
	Lexer lexer;

	Ast_Scope* internal_scope = NULL;
	Ast_Scope* current_scope = NULL;

	Parser (Ast_Scope* internal_scope) {
		this->internal_scope = internal_scope;
	}

	Ast_Scope* build_ast (const char* source_code) {
		this->lexer.set_input_text(source_code);

		auto global_scope = AST_NEW(Ast_Scope);
		global_scope->imports.push_back(this->internal_scope);

		this->scope(global_scope);

		return global_scope;
	}

	Ast_Scope* scope (Ast_Scope* scope = NULL) {
		if (!scope) {
			scope = AST_NEW(Ast_Scope, this->current_scope);
		}

		auto _tmp = this->current_scope;
		this->current_scope = scope;

		auto stm = this->statement();
		while (stm != NULL) {
			this->current_scope->add(stm);

			if (this->lexer.is_next(TOKEN_EOF)) {
				break;
			} else stm = this->statement();
		}

		this->current_scope = _tmp;
		return scope;
	}
	Ast_Statement* statement () {
		switch (this->lexer.peek()->type) {
			case TOKEN_EOF: {
				//this->report_expected("statement");
				return NULL;
			}
			case TOKEN_STM_END: {
				this->lexer.skip();
				return this->statement();
			}
			case TOKEN_BRAC_OPEN: {
				this->lexer.skip();
				auto _scope = this->scope();
				this->lexer.expect(TOKEN_BRAC_CLOSE);
				return _scope;
			}
			case TOKEN_IF: {
				return this->_if();
			}
			case TOKEN_WHILE: {
				this->lexer.skip();
				auto stm_while = AST_NEW(Ast_While);
				stm_while->condition = this->expression();
				stm_while->scope = this->scoped_statement();
				return stm_while;
			}
			case TOKEN_BREAK: {
				this->lexer.skip();
				this->lexer.try_skip(TOKEN_STM_END);
				return AST_NEW(Ast_Break);
			}
			case TOKEN_RETURN: {
				this->lexer.skip();
				auto output = AST_NEW(Ast_Return, this->expression());
				this->lexer.try_skip(TOKEN_STM_END);
				return output;
			}
			case TOKEN_IMPORT: {
				this->lexer.skip();
				auto import = AST_NEW(Ast_Import);

				import->include = this->lexer.try_skip(TOKEN_EXCLAMATION);

				auto literal = this->string_literal();
				auto new_length = strlen(literal->string_value) + 4;

				auto tmp = (char*) malloc(new_length);
				sprintf_s(tmp, new_length, "%s" DEFAULT_FILE_EXTENSION, literal->string_value);
				import->path = tmp;

				return import;
			}
			case TOKEN_FOREIGN: {
				this->lexer.skip();
				auto foreign = AST_NEW(Ast_Foreign);

				if (this->lexer.is_next(TOKEN_STRING)) {
					foreign->module_name = this->lexer.escaped_string();
				} else foreign->module_name = foreign->get_foreign_module_name_from_file();

				if (this->lexer.is_next(TOKEN_STRING)) {
					foreign->function_name = this->lexer.escaped_string();
				}

				if (foreign->function_name && this->lexer.is_next(TOKEN_BRAC_OPEN)) {
					//Logger::error_and_stop(&this->lexer.peek()->location, "foreign function name can only be used with scopes");
				}

				if (this->lexer.try_skip(TOKEN_BRAC_OPEN)) {
					// @TODO implement custom method instead of using scope()
					auto scope = this->scope();
					for (auto stm : scope->statements) {
						foreign->add(stm);
					}
					delete scope;
					this->lexer.expect(TOKEN_BRAC_CLOSE);
				} else foreign->add(this->declaration());

				return foreign;
			}
			case TOKEN_HASH: {
				this->lexer.skip();
				switch (this->lexer.peek()->type) {
					case TOKEN_IF: return AST_NEW(Ast_Static_If, this->_if());
					default: return AST_NEW(Ast_Run, this->expression());
				}
			}
			case TOKEN_ID: {
				if (this->lexer.peek(1)->type == TOKEN_COLON) {
					return this->declaration();
				}
			}
			default: {
				auto exp = this->expression();
				if (exp) {
					if (!this->lexer.try_skip(TOKEN_STM_END)) {
						return AST_NEW(Ast_Return, exp);
					} else return exp;
				} else return NULL;
			}
		}
	}

	Ast_If* _if () {
		if (this->lexer.is_next(TOKEN_IF)) {
			this->lexer.skip();

			auto stm_if = AST_NEW(Ast_If);
			stm_if->condition = this->expression();
			stm_if->then_scope = this->scoped_statement();
			if (this->lexer.try_skip(TOKEN_ELSE)) {
				stm_if->else_scope = this->scoped_statement();
			}

			return stm_if;
		} else {
			//Logger::error_and_stop(&this->lexer.peek()->location, "if-statement expected");
			return NULL;
		}
	}

	Ast_Scope* scoped_statement () {
		auto stm = this->statement();
		if (stm->stm_type != AST_STATEMENT_SCOPE) {
			auto scope = AST_NEW(Ast_Scope, this->current_scope);
			scope->add(stm);
			return scope;
		} else return static_cast<Ast_Scope*>(stm);
	}

	Ast_Declaration* declaration () {
		if(!this->lexer.is_next(TOKEN_ID)) return NULL;

		auto decl = AST_NEW(Ast_Declaration);
		decl->name = this->copy_token_text_and_skip();
		decl->scope = this->current_scope;

		this->lexer.expect(TOKEN_COLON);
		decl->type = this->type_instance();

		if (this->lexer.try_skip(TOKEN_COLON)) {
			decl->is_constant = true;
		} else this->lexer.try_skip(TOKEN_EQUAL);

		decl->expression = this->expression();

		this->lexer.try_skip(TOKEN_STM_END);

		return decl;
	}

	Ast_Declaration* declaration_or_type ();

	Ast_Expression* expression (short min_precedence = 1) {
		auto output = this->atom();
	    if (output != NULL) {
	        auto tt = this->lexer.peek()->type;
			auto precedence = Ast_Binary::get_precedence(tt);
			while (precedence >= min_precedence) {
				this->lexer.skip();

				auto next_min_precedence = precedence;
				if (Ast_Binary::is_left_associative(tt)) {
					next_min_precedence += 1;
				}

				Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
				_tmp->rhs = this->expression(next_min_precedence);
				_tmp->lhs = output;
				output = _tmp;

				if (tt == TOKEN_SQ_BRAC_OPEN) this->lexer.expect(TOKEN_SQ_BRAC_CLOSE);

				tt = this->lexer.peek()->type;
				precedence = Ast_Binary::get_precedence(tt);
			}
	    }
		return output;
	}

	Ast_Expression* atom () {
		if (this->lexer.is_next(TOKEN_ID)) {
			auto output = this->ident();
			if (this->lexer.try_skip(TOKEN_PAR_OPEN)) {
				auto call = AST_NEW(Ast_Function_Call);
				call->func = output;
				call->arguments = this->arguments();
				this->lexer.expect(TOKEN_PAR_CLOSE);
				return call;
			} else return output;
		} else if (this->lexer.try_skip(TOKEN_STRUCT)) {
			auto _struct = AST_NEW(Ast_Struct_Type);

			if (this->lexer.is_next(TOKEN_ID))
				_struct->name = this->copy_token_text_and_skip();

			if (this->lexer.is_next(TOKEN_BRAC_OPEN)) {
				this->lexer.skip();

				auto _scope = this->scope();
				for (auto stm : _scope->statements) {
					if (stm->stm_type == AST_STATEMENT_DECLARATION) {
						auto decl = static_cast<Ast_Declaration*>(stm);
						_struct->attributes.push_back(decl);
					} else {
						//Logger::error(stm, "Only declarations can go inside a struct");
					}
				}
				delete _scope;
				this->lexer.expect(TOKEN_BRAC_CLOSE);
			}

			return _struct;
		} else if (this->lexer.try_skip(TOKEN_FUNCTION)) {
			auto sub_scope = AST_NEW(Ast_Scope, this->current_scope);
			auto tmp = this->current_scope;
			this->current_scope = sub_scope;

			auto func_type = this->function_type();

			this->current_scope = tmp;

			if (this->lexer.try_skip(TOKEN_BRAC_OPEN)) {
				auto func = AST_NEW(Ast_Function);
				sub_scope->scope_of = func;
				func->scope = sub_scope;
				func->type = func_type;

				this->scope(func->scope);
				this->lexer.expect(TOKEN_BRAC_CLOSE);

				return func;
			} else return func_type;
		} else if (this->lexer.try_skip(TOKEN_CAST)) {
			auto cast = AST_NEW(Ast_Cast);
			this->lexer.expect(TOKEN_PAR_OPEN);
			cast->cast_to = this->type_instance();
			this->lexer.expect(TOKEN_PAR_CLOSE);
			cast->value = this->expression();
			return cast;
		} else if (this->lexer.try_skip(TOKEN_PAR_OPEN)) {
			auto result = this->expression();
			this->lexer.expect(TOKEN_PAR_CLOSE);
			return result;
		} else if (this->lexer.try_skip(TOKEN_HASH)) {
			this->lexer.skip();
			return AST_NEW(Ast_Run, this->expression());
		} else if (this->lexer.try_skip(TOKEN_FALSE)) {
			return ast_make_literal(false);
		} else if (this->lexer.try_skip(TOKEN_TRUE)) {
			return ast_make_literal(true);
		} else if (this->lexer.try_skip(TOKEN_MUL)) {
			return AST_NEW(Ast_Unary, AST_UNARY_REFERENCE, this->atom());
		} else if (this->lexer.try_skip(TOKEN_EXCLAMATION)) {
			return AST_NEW(Ast_Unary, AST_UNARY_NOT, this->atom());
		} else if (this->lexer.try_skip(TOKEN_SUB)) {
			return AST_NEW(Ast_Unary, AST_UNARY_NEGATE, this->atom());
		} else if (this->lexer.try_skip(TOKEN_AMP)) {
			return AST_NEW(Ast_Unary, AST_UNARY_DEREFERENCE, this->atom());
		} else if (this->lexer.try_skip(TOKEN_ADD)) {
			return this->expression();
		} else return this->literal();
	}

	Ast_Expression* type_instance () {
		if (this->lexer.try_skip(TOKEN_FUNCTION)) {
			return this->function_type();
		} else if (this->lexer.try_skip(TOKEN_MUL)) {
			return AST_NEW(Ast_Pointer_Type, this->type_instance());
		} else if (this->lexer.try_skip(TOKEN_SQ_BRAC_OPEN)) {
			if (this->lexer.try_skip(TOKEN_SQ_BRAC_CLOSE)) {
				return AST_NEW(Ast_Slice_Type, this->type_instance());
			} else {
				auto length = this->expression();
				this->lexer.expect(TOKEN_SQ_BRAC_CLOSE);

				auto _array = AST_NEW(Ast_Array_Type);
				_array->base = this->type_instance();
				_array->length = length;
				return _array;
			}
		} else {
			return this->ident();
		}
	}

	Ast_Function_Type* function_type () {
		auto fn_type = AST_NEW(Ast_Function_Type);

		if (this->lexer.try_skip(TOKEN_PAR_OPEN)) {
			while (!this->lexer.try_skip(TOKEN_PAR_CLOSE)) {
				auto decl = this->declaration();
				fn_type->arg_decls.push_back(decl);

				this->lexer.try_skip(TOKEN_COMMA);
			}
		}

		if (this->lexer.try_skip(TOKEN_ARROW)) {
			fn_type->ret_type = this->type_instance();
		} else fn_type->ret_type = Types::type_void;

		return fn_type;
	}

	Ast_Literal* literal () {
		Ast_Literal* output = NULL;
		switch (this->lexer.peek()->type) {
			case TOKEN_STRING: {
				output = this->string_literal();
				break;
			}
			case TOKEN_NUMBER: {
				output = AST_NEW(Ast_Literal);
				output->string_value = this->copy_token_text_and_skip();
				if (output->is_hexadecimal()) {
					output->literal_type = AST_LITERAL_UNSIGNED_INT;
					output->uint_value = strtoull(output->string_value + 2, NULL, 16);
				} else if (output->is_binary()) {
					output->literal_type = AST_LITERAL_UNSIGNED_INT;
					output->uint_value = strtoull(output->string_value + 2, NULL, 2);
				} else if (output->is_decimal()) {
					output->literal_type = AST_LITERAL_DECIMAL;
					output->decimal_value = atof(output->string_value);
				} else {
					output->literal_type = AST_LITERAL_UNSIGNED_INT;
					output->uint_value = strtoull(output->string_value, NULL, 10);
				}
				break;
			}
			default: break;
		}
		return output;
	}

	Ast_Literal* string_literal () {
		if (this->lexer.is_next(TOKEN_STRING)) {
			auto output = AST_NEW(Ast_Literal);
			output->string_value = this->lexer.escaped_string();
			output->literal_type = AST_LITERAL_STRING;
			return output;
		} else return NULL;
	}

	Ast_Arguments* arguments () {
		auto args = AST_NEW(Ast_Arguments);

		bool parsing_named = false;
		while (!this->lexer.is_next(TOKEN_PAR_CLOSE)) {
			auto exp = this->expression();

			auto last_is_named = args->add(exp);
			if (parsing_named && !last_is_named) {
				//Logger::error(exp, "All named parameters must be on the right part");
			} else parsing_named = last_is_named;

			this->lexer.try_skip(TOKEN_COMMA);
		}

		return args;
	}

	Ast_Ident* ident () {
		if (!this->lexer.is_next(TOKEN_ID)) return NULL;

		auto ident = AST_NEW(Ast_Ident);
		ident->name = this->copy_token_text_and_skip();

		// @Info this is the right time to do this, since on a non-constant
		// reference the declaration should already be in the scope.
		ident->declaration = this->current_scope->find_var_declaration(ident->name);

		return ident;
	}

	const char* copy_token_text_and_skip () {
		auto output = this->lexer.peek()->copy_text();
		this->lexer.skip();
		return output;
	}
};