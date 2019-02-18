#pragma once

#include "lexer/lexer.hpp"
#include "ast/nodes.hpp"
#include "ast/types.hpp"
#include "ast/factory.hpp"
#include "platform.hpp"

#define AST_NEW(T, ...) this->set_ast_location_info<T>(new T(__VA_ARGS__))

struct Parser {
	Lexer lexer;

	Ast_Scope* current_scope = NULL;

	void parse_into (Ast_Scope* scope, const char* absolute_path) {
		size_t length;
		auto source = os_read_entire_file(absolute_path, &length);
		this->parse_into(scope, source, length, absolute_path);
	}

	void parse_into (Ast_Scope* scope, const char* text, size_t length, const char* absolute_path) {
		this->lexer.set_source(text, length, absolute_path);

		char tmp_path[MAX_PATH_LENGTH];
		if (absolute_path) {
			os_get_current_directory(tmp_path);
			os_set_current_directory_path(absolute_path);
		}

		this->scope(scope);

		if (absolute_path) {
			os_set_current_directory(tmp_path);
		}
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
				stm_while->body = this->scoped_statement();
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
				output->scope = this->current_scope;
				this->lexer.try_skip(TOKEN_STM_END);
				return output;
			}
			case TOKEN_INCLUDE: {
				this->lexer.skip();
				auto import = AST_NEW(Ast_Import);
				import->scope = this->current_scope;
				import->is_include = true;
				this->string_literal_value(import->path);
				os_get_current_directory(import->current_folder);
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
					if (this->lexer.try_skip(TOKEN_EQUAL)) {
						auto assign = AST_NEW(Ast_Assign);
						assign->variable = exp;
						assign->value = this->expression();
						return assign;
					} else {
						this->lexer.expect(TOKEN_STM_END);
						return exp;
					}
				}
				return NULL;
			}
		}
	}

	Ast_If* _if () {
		if (this->lexer.is_next(TOKEN_IF)) {
			this->lexer.skip();

			auto stm_if = AST_NEW(Ast_If);
			stm_if->condition = this->expression();
			stm_if->then_body = this->scoped_statement();
			if (this->lexer.try_skip(TOKEN_ELSE)) {
				stm_if->else_body = this->scoped_statement();
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
		decl->type = this->type_expression();

		if (this->lexer.try_skip(TOKEN_COLON)) {
			decl->is_constant = true;
		} else this->lexer.try_skip(TOKEN_EQUAL);

		decl->expression = this->expression();
		this->bind(decl->name, decl->expression);

		this->lexer.try_skip(TOKEN_STM_END);

		return decl;
	}

	Ast_Expression* expression (short min_precedence = 1) {
		auto output = this->atom();
	    if (output != NULL) {
	        auto tt = this->lexer.peek()->type;
			auto precedence = get_operator_precedence(tt);
			while (precedence >= min_precedence) {
				this->lexer.skip();

				if (tt == TOKEN_PAR_OPEN) {
					auto call = AST_NEW(Ast_Function_Call);
					call->func = output;
					call->arguments = this->comma_separated_values();
					output = call;

					this->lexer.expect(TOKEN_PAR_CLOSE);
				} else if (tt == TOKEN_SQ_BRAC_OPEN) {
					Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
					_tmp->rhs = this->expression(precedence);
					_tmp->lhs = output;
					output = _tmp;

					this->lexer.expect(TOKEN_SQ_BRAC_CLOSE);
				} else if (tt == TOKEN_DOT) {
					if (!this->lexer.is_next(TOKEN_ID)) {
						this->lexer.report_expected("identifier");
					}

					Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
					_tmp->rhs = this->ident();
					_tmp->lhs = output;
					output = _tmp;
				} else {
					auto next_min_precedence = precedence;
					if (tt == TOKEN_EQUAL) {
						next_min_precedence += 1;
					}

					Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
					_tmp->rhs = this->expression(next_min_precedence);
					_tmp->lhs = output;
					output = _tmp;
				}

				tt = this->lexer.peek()->type;
				precedence = get_operator_precedence(tt);
			}
	    }
		return output;
	}

	Ast_Expression* atom () {
		if (this->lexer.is_next(TOKEN_ID)) {
			return this->ident();
		} else if (this->lexer.try_skip(TOKEN_STRUCT)) {
			auto _struct = AST_NEW(Ast_Struct_Type);

			if (this->lexer.is_next(TOKEN_ID))
				_struct->name = this->copy_token_text_and_skip();

			if (this->lexer.is_next(TOKEN_BRAC_OPEN)) {
				this->lexer.skip();
				_struct->scope.parent = this->current_scope;
				this->scope(&_struct->scope);
				this->lexer.expect(TOKEN_BRAC_CLOSE);
			}

			return _struct;
		} else if (this->lexer.try_skip(TOKEN_FUNCTION)) {
			auto sub_scope = AST_NEW(Ast_Scope, this->current_scope);
			auto tmp = this->current_scope;
			this->current_scope = sub_scope;

			auto arg_scope = AST_NEW(Ast_Scope, this->current_scope);
			auto ret_scope = AST_NEW(Ast_Scope, this->current_scope);
			auto func_type = this->function_type(arg_scope, ret_scope);

			this->current_scope = tmp;

			if (this->lexer.try_skip(TOKEN_BRAC_OPEN)) {
				auto func = AST_NEW(Ast_Function);
				sub_scope->scope_of = func;
				func->body = sub_scope;
				func->type = func_type;

				func->arg_scope = arg_scope;
				func->ret_scope = ret_scope;

				this->scope(func->body);
				this->lexer.expect(TOKEN_BRAC_CLOSE);

				return func;
			} else return func_type;
		} else if (this->lexer.try_skip(TOKEN_IMPORT)) {
			auto import = AST_NEW(Ast_Import);
			import->scope = this->current_scope;
			this->string_literal_value(import->path);
			os_get_current_directory(import->current_folder);
			return import;
		} else if (this->lexer.try_skip(TOKEN_CAST)) {
			auto cast = AST_NEW(Ast_Cast);
			this->lexer.expect(TOKEN_PAR_OPEN);
			cast->cast_to = this->type_expression();
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
			return AST_NEW(Ast_Unary, AST_UNARY_REFERENCE, this->expression());
		} else if (this->lexer.try_skip(TOKEN_EXCLAMATION)) {
			return AST_NEW(Ast_Unary, AST_UNARY_NOT, this->expression());
		} else if (this->lexer.try_skip(TOKEN_SUB)) {
			return AST_NEW(Ast_Unary, AST_UNARY_NEGATE, this->expression());
		} else if (this->lexer.try_skip(TOKEN_AMP)) {
			return AST_NEW(Ast_Unary, AST_UNARY_DEREFERENCE, this->expression());
		} else if (this->lexer.try_skip(TOKEN_ADD)) {
			return this->expression();
		} else return this->literal();
	}

	Ast_Expression* type_expression () {
		if (this->lexer.try_skip(TOKEN_FUNCTION)) {
			return this->function_type();
		} else if (this->lexer.try_skip(TOKEN_MUL)) {
			return AST_NEW(Ast_Pointer_Type, this->type_expression());
		} else if (this->lexer.try_skip(TOKEN_SQ_BRAC_OPEN)) {
			if (this->lexer.try_skip(TOKEN_SQ_BRAC_CLOSE)) {
				return AST_NEW(Ast_Slice_Type, this->type_expression());
			} else {
				auto length = this->expression();
				this->lexer.expect(TOKEN_SQ_BRAC_CLOSE);

 				auto _array = AST_NEW(Ast_Array_Type);
				_array->base = this->type_expression();
				_array->length = length;
				return _array;
			}
		} else return this->expression();
	}

	Ast_Function_Type* function_type (Ast_Scope* arg_scope = NULL, Ast_Scope* ret_scope = NULL) {
		auto fn_type = AST_NEW(Ast_Function_Type);

		if (this->lexer.try_skip(TOKEN_PAR_OPEN)) {
			while (!this->lexer.try_skip(TOKEN_PAR_CLOSE)) {
				auto decl = this->declaration();
				if (arg_scope) arg_scope->add(decl);

				fn_type->arg_types.push_back(decl->type);

				this->lexer.try_skip(TOKEN_COMMA);
			}
		}

		if (this->lexer.try_skip(TOKEN_ARROW)) {
			if (this->lexer.try_skip(TOKEN_PAR_OPEN)) {
				Ast_Declaration* decl = NULL;
				while (!this->lexer.try_skip(TOKEN_PAR_CLOSE)) {
					if (this->lexer.peek(0)->type == TOKEN_ID
							&& this->lexer.peek(1)->type == TOKEN_COLON) {
						decl = this->declaration();
					} else {
						decl = AST_NEW(Ast_Declaration);
						decl->scope = this->current_scope;
						decl->type = this->expression();
					}
					if (ret_scope) ret_scope->add(decl);
					fn_type->ret_types.push_back(decl->type);

					this->lexer.try_skip(TOKEN_COMMA);
				}
			} else {
				Ast_Declaration* decl = NULL;
				if (this->lexer.peek(0)->type == TOKEN_ID
						&& this->lexer.peek(1)->type == TOKEN_COLON) {
					decl = this->declaration();
				} else {
					decl = AST_NEW(Ast_Declaration);
					decl->scope = this->current_scope;
					decl->type = this->expression();
				}
				if (ret_scope) ret_scope->add(decl);
				fn_type->ret_types.push_back(decl->type);
			}
		} else {
			fn_type->ret_types.push_back(Types::type_void);
		}

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

	void string_literal_value (char* buffer) {
		auto literal = this->string_literal();
		memcpy(buffer, literal->string_value, strlen(literal->string_value) + 1);
		delete literal;
	}

	Ast_Arguments* comma_separated_values () {
		auto args = AST_NEW(Ast_Arguments);

		auto is_parsing = this->parse_next_expression_or_assignment(args);
		while (is_parsing && this->lexer.try_skip(TOKEN_COMMA)) {
			is_parsing = this->parse_next_expression_or_assignment(args);
		}

		return args;
	}

	bool parse_next_expression_or_assignment (Ast_Arguments* args) {
		auto exp = this->expression();
		if (exp) {
			if (this->lexer.try_skip(TOKEN_EQUAL)) {
				assert(exp->exp_type == AST_EXPRESSION_IDENT);
				auto ident = static_cast<Ast_Ident*>(exp);
				args->named[ident->name] = this->expression();
				delete ident;
			} else args->unnamed.push_back(exp);
			return true;
		} else return false;
	}

	Ast_Ident* ident () {
		if (!this->lexer.is_next(TOKEN_ID)) return NULL;

		auto ident = AST_NEW(Ast_Ident);
		ident->scope = this->current_scope;
		ident->name = this->copy_token_text_and_skip();

		// @Info this is the right time to do this, since on a non-constant
		// reference the declaration should already be in the scope.
		ident->declaration = this->current_scope->find_var_declaration(ident->name);

		return ident;
	}

	uint8_t get_operator_precedence (Token_Type token_type) {
		switch (token_type) {
			default: 		  			return 0;

	        case TOKEN_PAR_OPEN:        return 1;

			case TOKEN_SQ_BRAC_OPEN:    return 2;
			case TOKEN_DOUBLE_PIPE:		return 3;
			case TOKEN_DOUBLE_AMP:		return 4;
			case TOKEN_PIPE:			return 5;
			case TOKEN_TILDE:			return 6;
			case TOKEN_AMP:				return 7;

			case TOKEN_DOUBLE_EQUAL:
			case TOKEN_NOT_EQUAL:		return 8;

			case TOKEN_GREATER_EQUAL:
			case TOKEN_LESSER_EQUAL:
			case TOKEN_GREATER:
			case TOKEN_LESSER:			return 9;

			case TOKEN_RIGHT_SHIFT:
			case TOKEN_LEFT_SHIFT:		return 10;

			case TOKEN_ADD:
			case TOKEN_SUB:   			return 11;

			case TOKEN_MUL:
			case TOKEN_DIV:
			case TOKEN_PERCENT:			return 12;

			case TOKEN_CARET:
			case TOKEN_EXCLAMATION:		return 13;

			case TOKEN_DOUBLE_ADD:
			case TOKEN_DOUBLE_SUB:
			case TOKEN_DOT:   			return 14;
		}
	}

	void bind (const char* name, Ast_Expression* exp) {
		if (!exp) return;

		if (exp->exp_type == AST_EXPRESSION_FUNCTION) {
			auto func = static_cast<Ast_Function*>(exp);
			func->name = name;
		} else if (exp->exp_type == AST_EXPRESSION_TYPE) {
			auto type = static_cast<Ast_Type*>(exp);
			type->name = name;
		}
	}

	const char* copy_token_text_and_skip () {
		auto output = this->lexer.peek()->copy_text();
		this->lexer.skip();
		return output;
	}

	template<typename T>
	T* set_ast_location_info (T* ast_node) {
		ast_node->location.filename = this->lexer.source_path;
		ast_node->location.line = this->lexer.scanner.current_line;
		return ast_node;
	}
};
