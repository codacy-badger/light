#include "ast/parser.hpp"

#include "compiler.hpp"

#define AST_NEW(T, ...) this->factory->create_node<T>(__VA_ARGS__)

Lexer* Parser::setup (const char* filepath, Ast_Block* parent) {
	os_get_current_directory(this->last_path);
	os_set_current_directory_path(filepath);

	this->current_block = parent;
	return this->push_lexer(filepath);
}

void Parser::teardown () {
	this->all_lines += this->lexer->get_total_ancestor_lines();
	this->global_notes.clear();

	this->pop_lexer();

	os_set_current_directory(this->last_path);
}

Lexer* Parser::push_lexer(const char* filepath) {
	auto tmp = this->lexer;
	this->lexer = new Lexer(filepath, this->lexer);
	this->factory->lexer = this->lexer;
	return tmp;
}

void Parser::pop_lexer() {
	auto tmp = this->lexer;
	this->lexer = tmp->parent;
	this->factory->lexer = this->lexer;
	delete tmp;
}

void Parser::add (Ast_Statement* stm) {
	if (this->global_notes.size()) {
		stm->notes.insert(stm->notes.end(),
			this->global_notes.begin(), this->global_notes.end());
	}

	this->current_block->list.push_back(stm);
}

void Parser::block (Ast_Block* inner_block) {
	auto _tmp = this->current_block;
	this->current_block = inner_block;

	auto stm = this->statement();
	while (stm != NULL) {
		this->add(stm);

		if (this->lexer->is_next_type(TOKEN_EOF)) break;
		else stm = this->statement();
	}

	this->current_block = _tmp;
}

Ast_Note* Parser::note () {
	if (this->lexer->optional_skip(TOKEN_HASH)) {
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
		case TOKEN_IMPORT: {
			this->lexer->skip();
			auto import = AST_NEW(Ast_Import, this->expression());
			this->lexer->optional_skip(TOKEN_STM_END);
			return import;
		}
		case TOKEN_HASH: {
			auto note = this->note();
			if (note->is_global) {
				if (strcmp(note->name, "end") == 0) {
					this->global_notes.clear();
					delete note;
				} else {
					this->global_notes.push_back(note);
				}
			} else notes.push_back(note);

			auto stm = this->statement();
			if (stm) {
				stm->notes.insert(stm->notes.end(), this->notes.begin(), this->notes.end());
				this->notes.clear();
			}
			return stm;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip();
			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return _block;
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
			output->exp = this->expression();
			output->block = this->current_block;
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
						output->block = this->current_block;
						output->exp = exp;
						return output;
					}
				} else return NULL;
			}
		}
	}
}

Ast_Declaration* Parser::declaration (Ast_Ident* ident) {
	if (!ident) ident = this->ident();
	if (!ident) return NULL;

	auto decl = AST_NEW(Ast_Declaration);
	decl->scope = this->current_block;
	decl->name = ident->name;

	auto other = this->current_block->find_declaration_in_same_scope(decl->name);
	if (other) {
		report_error(&decl->location, "re-declaration of variable or constant '%s'", decl->name);
		report_error_and_stop(&other->location, "previous declaration here");
	}

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
	Ast_Expression* output = this->_atom(initial);
    if (output != NULL) {
        Token_Type tt = this->lexer->next_type;
		auto precedence = Ast_Binary::get_precedence(tt);
		while (precedence >= min_precedence) {
			this->lexer->skip();

			short nextMinPrec = precedence;
			if (Ast_Binary::is_left_associative(tt)) nextMinPrec += 1;

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, nextMinPrec);
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

			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			for (auto stm : _block->list) {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					decl->_struct = _struct;
					_struct->attributes.push_back(decl);
				} else {
					report_error_and_stop(&stm->location, "Only declarations can go inside a struct");
				}
			}
			delete _block;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		auto sub_scope = AST_NEW(Ast_Block, this->current_block);
		auto tmp = this->current_block;
		this->current_block = sub_scope;

		auto func_type = this->function_type();

		this->current_block = tmp;

		if (this->lexer->optional_skip(TOKEN_BRAC_OPEN)) {
			auto func = AST_NEW(Ast_Function);
			sub_scope->scope_of = func;
			func->scope = sub_scope;
			func->type = func_type;

			this->block(func->scope);
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
	} else {
		auto lit = this->literal();
		return lit ? lit : this->type_instance();
	}
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
			auto decl = this->current_block->find_const_declaration(ident->name);
			if (decl && decl->expression
					&& decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
				this->factory->delete_node(ident);
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
	} else fn_type->ret_type = Compiler::instance->types->type_def_void;

	return fn_type;
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->lexer->next_type) {
		case TOKEN_STRING: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_STRING;
			output->string_value = this->lexer->text();
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			auto number_str = this->lexer->text();
			if (number_str[0] == '0' && number_str[1] == 'x') {
				output->literal_type = AST_LITERAL_UNSIGNED_INT;
				output->uint_value = strtoull(number_str + 2, NULL, 16);
			} else if (number_str[0] == '0' && number_str[1] == 'b') {
				output->literal_type = AST_LITERAL_UNSIGNED_INT;
				output->uint_value = strtoull(number_str + 2, NULL, 2);
			} else if (strstr(number_str, ".") != NULL) {
				output->literal_type = AST_LITERAL_DECIMAL;
				output->decimal_value = atof(number_str);
			} else {
				output->literal_type = AST_LITERAL_UNSIGNED_INT;
				output->uint_value = strtoull(number_str, NULL, 10);
			}
			break;
		}
		default: break;
	}
	return output;
}

void Parser::comma_separated_arguments (vector<Ast_Expression*>* arguments) {
	auto exp = this->expression();
	if (exp) {
		while (exp != NULL) {
			arguments->push_back(exp);
			this->lexer->optional_skip(TOKEN_COMMA);
			exp = this->expression();
		}
	}
}

Ast_Ident* Parser::ident (const char* name) {
	if (!name && !this->lexer->is_next_type(TOKEN_ID)) return NULL;

	auto ident = AST_NEW(Ast_Ident, this->current_block);
	ident->name = name ? name : this->lexer->text();

	// this is the right time to do this, since on a non-constant reference
	// the declaration should already be in the scope.
	ident->declaration = this->current_block->find_non_const_declaration(ident->name);
	return ident;
}

void Parser::print_pipe_metrics () {
	PRINT_METRIC("Lines of Code:         %zd", this->all_lines);
	PRINT_METRIC("AST nodes created:     %zd", this->factory->node_count);
}
