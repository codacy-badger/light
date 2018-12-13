#include "ast/parser.hpp"

#include "compiler.hpp"

uint64_t Ast_Factory::node_count = 0;

#define AST_NEW(T, ...) Ast_Factory::create_node<T>(&this->lexer->buffer->location, __VA_ARGS__)

Ast_Scope* Parser::run (const char* filepath, Ast_Scope* parent) {
	auto start = os_get_user_time();

	this->current_scope = parent;
	auto tmp_lexer = this->lexer;
	this->lexer = new Lexer(filepath);

	auto global_scope = AST_NEW(Ast_Scope, parent);
	global_scope->is_global = true;
	this->scope(global_scope);

	this->all_lines += this->lexer->get_total_ancestor_lines();
	this->notes->clear_global();

	delete this->lexer;
	this->lexer = tmp_lexer;

	this->time += os_time_user_stop(start);

	return global_scope;
}

void Parser::push (Ast_Statement* stm) {
	this->notes->push_global_into(stm);
	this->current_scope->statements.push_back(stm);
}

Ast_Scope* Parser::scope (Ast_Scope* inner_scope) {
	if (!inner_scope) {
		inner_scope = AST_NEW(Ast_Scope, this->current_scope);
	}

	auto _tmp = this->current_scope;
	this->current_scope = inner_scope;

	auto stm = this->statement();
	while (stm != NULL) {
		this->push(stm);

		if (this->lexer->is_next_type(TOKEN_EOF)) {
			break;
		} else stm = this->statement();
	}

	this->current_scope = _tmp;
	return inner_scope;
}

Ast_Note* Parser::note () {
	if (this->lexer->optional_skip(TOKEN_AT)) {
		auto note = AST_NEW(Ast_Note);
		note->is_global = this->lexer->optional_skip(TOKEN_EXCLAMATION);

		if (this->lexer->is_next_type(TOKEN_ID)) {
			note->name = this->lexer->text();
		} else report_error_and_stop(&note->location, "All notes must have a name");

		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			this->comma_separated_arguments(&note->arguments);
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
		}
		return note;
	} else return NULL;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->next_type) {
		case TOKEN_EOF: return NULL;
		case TOKEN_STM_END: {
			this->lexer->skip();
			return this->statement();
		}
		case TOKEN_HASH: {
			this->lexer->skip();
			return this->directive();
		}
		case TOKEN_IMPORT: {
			this->lexer->skip();
			auto import = AST_NEW(Ast_Import, this->expression());
			this->lexer->optional_skip(TOKEN_STM_END);
			return import;
		}
		case TOKEN_AT: {
			auto note = this->note();
			while (note != NULL) {
				this->notes->push(note);
				note = this->note();
			}

			auto stm = this->statement();
			this->notes->push_into(stm);
			return stm;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip();
			auto _scope = this->scope();
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return _scope;
		}
		case TOKEN_IF: {
			this->lexer->skip();
			auto stm_if = AST_NEW(Ast_If);
			stm_if->condition = this->expression();
			stm_if->then_statement = this->statement();
			if (this->lexer->optional_skip(TOKEN_ELSE)) {
				stm_if->else_statement = this->statement();
			}
			return stm_if;
		}
		case TOKEN_WHILE: {
			this->lexer->skip();
			auto stm_while = AST_NEW(Ast_While);
			stm_while->condition = this->expression();
			stm_while->statement = this->statement();
			return stm_while;
		}
		case TOKEN_BREAK: {
			this->lexer->skip();
			this->lexer->optional_skip(TOKEN_STM_END);
			return AST_NEW(Ast_Break);
		}
		case TOKEN_RETURN: {
			this->lexer->skip();
			auto output = AST_NEW(Ast_Return);
			output->expression = this->expression();
			output->scope = this->current_scope;
			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		default: {
			auto ident = this->ident();
			if (this->lexer->is_next_type(TOKEN_COLON)) {
				return this->declaration(ident);
			} else {
				auto exp = this->expression(ident);
				if (exp) {
					if (this->lexer->optional_skip(TOKEN_STM_END)) {
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

Ast_Directive* Parser::directive () {
	switch (this->lexer->next_type) {
		case TOKEN_IMPORT: {
			this->lexer->skip();
			auto import = AST_NEW(Ast_Directive_Import);

			auto literal = this->literal();
			if (literal->literal_type == AST_LITERAL_STRING) {
				import->path = literal->string_value;
				os_get_absolute_path(import->path, import->absolute_path);
			} else ERROR_STOP(literal, "Expected string literal after import");

			return import;
		}
		case TOKEN_INCLUDE: {
			this->lexer->skip();
			auto include = AST_NEW(Ast_Directive_Include);

			auto literal = this->literal();
			if (literal->literal_type == AST_LITERAL_STRING) {
				include->path = literal->string_value;
				os_get_absolute_path(include->path, include->absolute_path);
			} else ERROR_STOP(literal, "Expected string literal after include");

			return include;
		}
			case TOKEN_IF: {
				this->lexer->skip();
				auto directive_if = AST_NEW(Ast_Directive_If);

				directive_if->stm_if = AST_NEW(Ast_If);
				directive_if->stm_if->condition = this->expression();
				directive_if->stm_if->then_statement = this->statement();

				if (this->lexer->optional_skip(TOKEN_ELSE)) {
					directive_if->stm_if->else_statement = this->statement();
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

	if (this->lexer->check_skip(TOKEN_COLON)) {
		decl->type = this->type_instance();
	}

	if (this->lexer->optional_skip(TOKEN_COLON)) {
		decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
	} else this->lexer->optional_skip(TOKEN_EQUAL);

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

	this->lexer->optional_skip(TOKEN_STM_END);

	return decl;
}

Ast_Expression* Parser::expression (Ast_Ident* initial, short min_precedence) {
	auto output = this->_atom(initial);
    if (output != NULL) {
        auto tt = this->lexer->next_type;
		auto precedence = Ast_Binary::get_precedence(tt);
		while (precedence >= min_precedence) {
			this->lexer->skip();

			auto next_min_precedence = precedence;
			if (Ast_Binary::is_left_associative(tt)) {
				next_min_precedence += 1;
			}

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, next_min_precedence);
			_tmp->lhs = output;
			output = _tmp;

			if (tt == TOKEN_SQ_BRAC_OPEN) this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);

			tt = this->lexer->next_type;
			precedence = Ast_Binary::get_precedence(tt);
		}
    }
	return output;
}

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->lexer->is_next_type(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			auto call = AST_NEW(Ast_Function_Call);
			call->func = output;
			this->comma_separated_arguments(&call->arguments);
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
			return call;
		} else return output;
	} else if (this->lexer->optional_skip(TOKEN_STRUCT)) {
		auto _struct = AST_NEW(Ast_Struct_Type);

		if (this->lexer->is_next_type(TOKEN_ID))
			_struct->name = this->lexer->text();

		if (this->lexer->is_next_type(TOKEN_BRAC_OPEN)) {
			this->lexer->skip();

			auto _scope = this->scope();
			for (auto stm : _scope->statements) {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					decl->_struct = _struct;
					_struct->attributes.push_back(decl);
				} else {
					report_error_and_stop(&stm->location, "Only declarations can go inside a struct");
				}
			}
			delete _scope;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		auto sub_scope = AST_NEW(Ast_Scope, this->current_scope);
		auto tmp = this->current_scope;
		this->current_scope = sub_scope;

		auto func_type = this->function_type();

		this->current_scope = tmp;

		if (this->lexer->optional_skip(TOKEN_BRAC_OPEN)) {
			auto func = AST_NEW(Ast_Function);
			sub_scope->scope_of = func;
			func->scope = sub_scope;
			func->type = func_type;

			this->scope(func->scope);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);

			return func;
		} else return func_type;
	} else if (this->lexer->optional_skip(TOKEN_CAST)) {
		auto cast = AST_NEW(Ast_Cast);
		this->lexer->check_skip(TOKEN_PAR_OPEN);
		cast->cast_to = this->type_instance();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		cast->value = this->expression();
		return cast;
	} else if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->optional_skip(TOKEN_FALSE)) {
		return Types::value_false;
	} else if (this->lexer->optional_skip(TOKEN_TRUE)) {
		return Types::value_true;
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		return AST_NEW(Ast_Unary, AST_UNARY_REFERENCE, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_EXCLAMATION)) {
		return AST_NEW(Ast_Unary, AST_UNARY_NOT, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_SUB)) {
		return AST_NEW(Ast_Unary, AST_UNARY_NEGATE, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_AMP)) {
		return AST_NEW(Ast_Unary, AST_UNARY_DEREFERENCE, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_ADD)) {
		return this->expression();
	} else return this->literal();
}

Ast_Expression* Parser::type_instance () {
	if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		return this->function_type();
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		auto base_type = this->type_instance();
		return Compiler::instance->types->get_pointer_type(base_type);
	} else if (this->lexer->optional_skip(TOKEN_SQ_BRAC_OPEN)) {
		auto length = this->expression();
		if (length) {
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();

			auto _array = AST_NEW(Ast_Array_Type);
			_array->length = length;
			_array->base = base_type;
			return _array;
		} else {
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();
			return Compiler::instance->types->get_slice_type(base_type);
		}
	} else {
		auto ident = this->ident();
		if (ident) {
			auto decl = this->current_scope->find_const_declaration(ident->name);
			if (decl && decl->expression && decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
				Ast_Factory::delete_node(ident);
				return decl->expression;
			} else return ident;
		} else return NULL;
	}
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);

	if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		auto decl = this->declaration();
		while (decl != NULL) {
			fn_type->arg_decls.push_back(decl);

			if (!this->lexer->optional_skip(TOKEN_COMMA)) break;
			decl = this->declaration();
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}

	if (this->lexer->optional_skip(TOKEN_ARROW)) {
		fn_type->ret_type = this->type_instance();
	} else fn_type->ret_type = Types::type_void;

	return fn_type;
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->lexer->next_type) {
		case TOKEN_STRING: {
			output = AST_NEW(Ast_Literal);
			output->string_value = this->lexer->text();
			output->literal_type = AST_LITERAL_STRING;
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			output->string_value = this->lexer->text();
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

void Parser::comma_separated_arguments (vector<Ast_Expression*>* arguments) {
	auto exp = this->expression();
	while (exp != NULL) {
		arguments->push_back(exp);

		this->lexer->optional_skip(TOKEN_COMMA);
		exp = this->expression();
	}
}

Ast_Ident* Parser::ident () {
	if (!this->lexer->is_next_type(TOKEN_ID)) return NULL;

	auto ident = AST_NEW(Ast_Ident, this->current_scope);
	ident->name = this->lexer->text();

	// this is the right time to do this, since on a non-constant reference
	// the declaration should already be in the scope.
	ident->declaration = this->current_scope->find_non_const_declaration(ident->name);
	return ident;
}

void Parser::print_metrics (double total_time) {
	double percent = (this->time * 100.0) / total_time;
	printf("  - %-25s%8.6fs (%5.2f%%)\n", "Lexer & Parser",
		this->time, percent);
	PRINT_METRIC("Lines of Code:         %zd", this->all_lines);
	PRINT_METRIC("Nodes created:     %zd", Ast_Factory::node_count);
}
