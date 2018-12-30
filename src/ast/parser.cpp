#include "ast/parser.hpp"

#include "compiler.hpp"

#define DEFAULT_FILE_EXTENSION ".li"

#define AST_NEW(T, ...) this->factory->create<T>(&this->peek()->location, __VA_ARGS__)

Token* Parser::eof = new Token(NULL, TOKEN_EOF);

Ast_Scope* Parser::build_ast (Ast_Scope* parent) {
	auto start = os_get_user_time();

	auto global_scope = AST_NEW(Ast_Scope, parent);
	global_scope->is_global = true;

	this->index = 0;
	this->scope(global_scope);
	for (auto token : this->tokens)
		delete token;
	this->tokens.clear();

	this->total_time += os_time_user_stop(start);
	return global_scope;
}

Token* Parser::peek (size_t offset) {
	auto new_index = this->index + offset;
	if (new_index < this->tokens.size()) {
		return this->tokens[new_index];
	} else return Parser::eof;
}

Token* Parser::next () {
	if (this->index < this->tokens.size()) {
		return this->tokens[this->index++];
	} else return Parser::eof;
}

bool Parser::is_next (Token_Type type) {
	return this->peek()->type == type;
}

void Parser::skip (size_t offset) {
	this->index += offset;
}

bool Parser::try_skip (Token_Type type) {
	if (this->is_next(type)) {
		this->index++;
		return true;
	} else return false;
}

bool Parser::expect (Token_Type type) {
	if (!this->try_skip(type)) {
		// TODO: report unexpected token
		return false;
	} else return true;
}

Ast_Scope* Parser::scope (Ast_Scope* inner_scope) {
	if (!inner_scope) {
		inner_scope = AST_NEW(Ast_Scope, this->current_scope);
	}

	auto _tmp = this->current_scope;
	this->current_scope = inner_scope;

	auto stm = this->statement();
	while (stm != NULL) {
		this->current_scope->add(stm);

		if (this->is_next(TOKEN_EOF)) {
			break;
		} else stm = this->statement();
	}

	this->current_scope = _tmp;
	return inner_scope;
}

const char* Parser::note () {
	if (this->try_skip(TOKEN_AT)) {
		return this->next()->text;
	} else return NULL;
}

Ast_Statement* Parser::statement () {
	switch (this->peek()->type) {
		case TOKEN_EOF: return NULL;
		case TOKEN_STM_END: {
			this->skip();
			return this->statement();
		}
		case TOKEN_AT: {
			vector<const char*> _notes;

			auto note = this->note();
			while (note != NULL) {
				_notes.push_back(note);
				note = this->note();
			}

			auto stm = this->statement();
			if (stm) stm->notes = _notes;
			return stm;
		}
		case TOKEN_BRAC_OPEN: {
			this->skip();
			auto _scope = this->scope();
			this->expect(TOKEN_BRAC_CLOSE);
			return _scope;
		}
		case TOKEN_IF: {
			this->skip();
			auto stm_if = AST_NEW(Ast_If);
			stm_if->condition = this->expression();
			stm_if->then_scope = this->scoped_statement();
			if (this->try_skip(TOKEN_ELSE)) {
				stm_if->else_scope = this->scoped_statement();
			}
			return stm_if;
		}
		case TOKEN_WHILE: {
			this->skip();
			auto stm_while = AST_NEW(Ast_While);
			stm_while->condition = this->expression();
			stm_while->scope = this->scoped_statement();
			return stm_while;
		}
		case TOKEN_BREAK: {
			this->skip();
			this->try_skip(TOKEN_STM_END);
			return AST_NEW(Ast_Break);
		}
		case TOKEN_RETURN: {
			this->skip();
			auto output = AST_NEW(Ast_Return);
			output->expression = this->expression();
			output->scope = this->current_scope;
			this->try_skip(TOKEN_STM_END);
			return output;
		}
		default: {
			auto ident = this->ident();
			if (this->is_next(TOKEN_COLON)) {
				return this->declaration(ident);
			} else {
				auto exp = this->expression(ident);
				if (exp) {
					if (this->try_skip(TOKEN_STM_END)) {
						return exp;
					} else {
						auto output = AST_NEW(Ast_Return);
						output->scope = this->current_scope;
						output->expression = exp;
						return output;
					}
				} else return NULL;
			}
		}
	}
}

Ast_Scope* Parser::scoped_statement () {
	auto stm = this->statement();
	if (stm->stm_type != AST_STATEMENT_SCOPE) {
		auto scope = AST_NEW(Ast_Scope, this->current_scope);
		scope->add(stm);
		return scope;
	} else return static_cast<Ast_Scope*>(stm);
}

Ast_Directive* Parser::directive () {
	switch (this->peek()->type) {
		case TOKEN_IMPORT: {
			this->skip();
			auto import = AST_NEW(Ast_Directive_Import);

			auto literal = this->string_literal();
			auto new_length = strlen(literal->string_value) + 4;

			auto tmp = (char*) malloc(new_length);
			sprintf_s(tmp, new_length, "%s.li", literal->string_value);
			import->path = tmp;

			return import;
		}
		case TOKEN_INCLUDE: {
			this->skip();
			auto include = AST_NEW(Ast_Directive_Include);

			auto literal = this->string_literal();
			auto new_length = strlen(literal->string_value) + 4;

			auto tmp = (char*) malloc(new_length);
			sprintf_s(tmp, new_length, "%s.li", literal->string_value);
			include->path = tmp;

			return include;
		}
		case TOKEN_RUN: {
			this->skip();
			auto run = AST_NEW(Ast_Directive_Run);
			run->expression = this->expression();
			return run;
		}
		case TOKEN_FOREIGN: {
			this->skip();
			auto foreign = AST_NEW(Ast_Directive_Foreign);

			if (this->peek()->type == TOKEN_STRING) {
				foreign->module_name = this->next()->text;
			} else foreign->module_name = foreign->get_foreign_module_name_from_file();

			if (this->peek()->type == TOKEN_STRING) {
				foreign->function_name = this->next()->text;
			}

			if (this->try_skip(TOKEN_BRAC_OPEN)) {
				auto scope = this->scope();
				for (auto stm : scope->statements) {
					foreign->add(stm);
				}
				this->expect(TOKEN_BRAC_CLOSE);
			} else foreign->add(this->declaration());

			return foreign;
		}
		case TOKEN_IF: {
			this->skip();
			auto directive_if = AST_NEW(Ast_Directive_If);

			directive_if->stm_if = AST_NEW(Ast_If);
			directive_if->stm_if->condition = this->expression();
			directive_if->stm_if->then_scope = this->scoped_statement();

			if (this->try_skip(TOKEN_ELSE)) {
				directive_if->stm_if->else_scope = this->scoped_statement();
			}

			return directive_if;
		}
		default: return NULL;
	}
}

Ast_Declaration* Parser::declaration (Ast_Ident* ident) {
	if (!ident) ident = this->ident();
	if (!ident) return NULL;

	auto decl = AST_NEW(Ast_Declaration);
	decl->scope = this->current_scope;
	decl->name = ident->name;

	if (this->expect(TOKEN_COLON)) {
		decl->type = this->type_instance();
	}

	if (this->try_skip(TOKEN_COLON)) {
		decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
	} else this->try_skip(TOKEN_EQUAL);

	decl->expression = this->expression();

	if (decl->expression && decl->is_constant()) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			auto fn = static_cast<Ast_Function*>(decl->expression);
			if (!fn->name) fn->name = decl->name;
		} else if (decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
			auto defn_ty = static_cast<Ast_Type_Instance*>(decl->expression);
			if (defn_ty->typedef_type == AST_TYPEDEF_STRUCT) {
				auto _struct = static_cast<Ast_Struct_Type*>(defn_ty);
				if (!_struct->name) _struct->name = _strdup(decl->name);
			}
		}
	}

	this->try_skip(TOKEN_STM_END);

	return decl;
}

Ast_Expression* Parser::expression (Ast_Ident* initial, short min_precedence) {
	auto output = this->_atom(initial);
    if (output != NULL) {
        auto tt = this->peek()->type;
		auto precedence = Ast_Binary::get_precedence(tt);
		while (precedence >= min_precedence) {
			this->skip();

			auto next_min_precedence = precedence;
			if (Ast_Binary::is_left_associative(tt)) {
				next_min_precedence += 1;
			}

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, next_min_precedence);
			_tmp->lhs = output;
			output = _tmp;

			if (tt == TOKEN_SQ_BRAC_OPEN) this->expect(TOKEN_SQ_BRAC_CLOSE);

			tt = this->peek()->type;
			precedence = Ast_Binary::get_precedence(tt);
		}
    }
	return output;
}

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->is_next(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->try_skip(TOKEN_PAR_OPEN)) {
			auto call = AST_NEW(Ast_Function_Call);
			call->func = output;
			call->arguments = this->arguments();
			this->expect(TOKEN_PAR_CLOSE);
			return call;
		} else return output;
	} else if (this->try_skip(TOKEN_STRUCT)) {
		auto _struct = AST_NEW(Ast_Struct_Type);

		if (this->is_next(TOKEN_ID))
			_struct->name = this->next()->text;

		if (this->is_next(TOKEN_BRAC_OPEN)) {
			this->skip();

			auto _scope = this->scope();
			for (auto stm : _scope->statements) {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					_struct->attributes.push_back(decl);
				} else {
					report_error_and_stop(&stm->location, "Only declarations can go inside a struct");
				}
			}
			delete _scope;
			this->expect(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->try_skip(TOKEN_FUNCTION)) {
		auto sub_scope = AST_NEW(Ast_Scope, this->current_scope);
		auto tmp = this->current_scope;
		this->current_scope = sub_scope;

		auto func_type = this->function_type();

		this->current_scope = tmp;

		if (this->try_skip(TOKEN_BRAC_OPEN)) {
			auto func = AST_NEW(Ast_Function);
			sub_scope->scope_of = func;
			func->scope = sub_scope;
			func->type = func_type;

			this->scope(func->scope);
			this->expect(TOKEN_BRAC_CLOSE);

			return func;
		} else return func_type;
	} else if (this->try_skip(TOKEN_CAST)) {
		auto cast = AST_NEW(Ast_Cast);
		this->expect(TOKEN_PAR_OPEN);
		cast->cast_to = this->type_instance();
		this->expect(TOKEN_PAR_CLOSE);
		cast->value = this->expression();
		return cast;
	} else if (this->try_skip(TOKEN_PAR_OPEN)) {
		auto result = this->expression();
		this->expect(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->try_skip(TOKEN_HASH)) {
		return this->directive();
	} else if (this->try_skip(TOKEN_FALSE)) {
		return ast_make_literal(false);
	} else if (this->try_skip(TOKEN_TRUE)) {
		return ast_make_literal(true);
	} else if (this->try_skip(TOKEN_MUL)) {
		return AST_NEW(Ast_Unary, AST_UNARY_REFERENCE, this->_atom());
	} else if (this->try_skip(TOKEN_EXCLAMATION)) {
		return AST_NEW(Ast_Unary, AST_UNARY_NOT, this->_atom());
	} else if (this->try_skip(TOKEN_SUB)) {
		return AST_NEW(Ast_Unary, AST_UNARY_NEGATE, this->_atom());
	} else if (this->try_skip(TOKEN_AMP)) {
		return AST_NEW(Ast_Unary, AST_UNARY_DEREFERENCE, this->_atom());
	} else if (this->try_skip(TOKEN_ADD)) {
		return this->expression();
	} else return this->literal();
}

Ast_Expression* Parser::type_instance () {
	if (this->try_skip(TOKEN_FUNCTION)) {
		return this->function_type();
	} else if (this->try_skip(TOKEN_MUL)) {
		auto base_type = this->type_instance();
		return Compiler::inst->types->get_pointer_type(base_type);
	} else if (this->try_skip(TOKEN_SQ_BRAC_OPEN)) {
		auto length = this->expression();
		if (length) {
			this->expect(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();

			auto _array = AST_NEW(Ast_Array_Type);
			_array->length = length;
			_array->base = base_type;
			return _array;
		} else {
			this->expect(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();
			return Compiler::inst->types->get_slice_type(base_type);
		}
	} else {
		auto ident = this->ident();
		if (ident) {
			auto decl = this->current_scope->find_const_declaration(ident->name);
			if (decl && decl->expression && decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
				this->factory->destroy(ident);
				return decl->expression;
			} else return ident;
		} else return NULL;
	}
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);

	if (this->try_skip(TOKEN_PAR_OPEN)) {
		auto decl = this->declaration();
		while (decl != NULL) {
			fn_type->arg_decls.push_back(decl);

			if (!this->try_skip(TOKEN_COMMA)) break;
			decl = this->declaration();
		}
		this->expect(TOKEN_PAR_CLOSE);
	}

	if (this->try_skip(TOKEN_ARROW)) {
		fn_type->ret_type = this->type_instance();
	} else fn_type->ret_type = Types::type_void;

	return fn_type;
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->peek()->type) {
		case TOKEN_STRING: {
			output = this->string_literal();
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			output->string_value = this->next()->text;
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

Ast_Literal* Parser::string_literal () {
	if (this->peek()->type == TOKEN_STRING) {
		auto output = AST_NEW(Ast_Literal);
		output->string_value = this->next()->text;
		output->literal_type = AST_LITERAL_STRING;
		return output;
	} else return NULL;
}

Ast_Arguments* Parser::arguments () {
	auto args = AST_NEW(Ast_Arguments);

	bool parsing_named = false;
	auto exp = this->expression();
	while (exp != NULL) {
		auto last_is_named = args->add(exp);
		if (parsing_named && !last_is_named) {
			ERROR_STOP(exp, "All named parameters must be on the right part");
		} else parsing_named = last_is_named;

		this->try_skip(TOKEN_COMMA);
		exp = this->expression();
	}

	return args;
}

Ast_Ident* Parser::ident () {
	if (!this->is_next(TOKEN_ID)) return NULL;

	auto ident = AST_NEW(Ast_Ident, this->current_scope);
	ident->name = this->next()->text;

	// this is the right time to do this, since on a non-constant reference
	// the declaration should already be in the scope.
	ident->declaration = this->current_scope->find_non_const_declaration(ident->name);
	return ident;
}

void Parser::print_metrics (double user_interval) {
	double percent = (this->total_time * 100.0) / user_interval;
	printf("  - %-25s%8.6fs (%5.2f%%)\n", "Parser", this->total_time, percent);
	PRINT_METRIC("AST Nodes created:     %zd", this->factory->node_count);
}
