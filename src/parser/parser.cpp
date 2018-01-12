#include "parser/parser.hpp"

#include "compiler.hpp"

Ast* setup_ast_node (Lexer* lexer, Ast* node) {
	if (lexer) node->location = lexer->buffer->location;
	return node;
}

#define AST_NEW(T, ...) ((T*)setup_ast_node(lexer, new T(__VA_ARGS__)))

#define SET_PATH(other) if (strcmp(other, this->current_path) != 0) {			\
	os_set_current_directory(other);											\
	strcpy_s(this->current_path, MAX_PATH_LENGTH, other); }

#define DECL_TYPE(type) this->add(ast_make_declaration(type->name, type), parent);

Ast_Block* Parser::run (const char* filepath, Ast_Block* parent) {
	char* file_part = NULL;
	auto abs_path = (char*) malloc(MAX_PATH_LENGTH);
	os_get_absolute_path(filepath, abs_path, &file_part);
	this->imported_files.push_back(abs_path);

	char last_path[MAX_PATH_LENGTH];
	strcpy_s(last_path, MAX_PATH_LENGTH, this->current_path);

	auto _c = *file_part;
	*file_part = '\0';
	SET_PATH(abs_path);
	*file_part = _c;

	if (!parent) {
		parent = AST_NEW(Ast_Block);
		parent->is_global = true;

		DECL_TYPE(g_compiler->type_def_type);
		DECL_TYPE(g_compiler->type_def_void);
		DECL_TYPE(g_compiler->type_def_bool);
		DECL_TYPE(g_compiler->type_def_s8);
		DECL_TYPE(g_compiler->type_def_s16);
		DECL_TYPE(g_compiler->type_def_s32);
		DECL_TYPE(g_compiler->type_def_s64);
		DECL_TYPE(g_compiler->type_def_u8);
		DECL_TYPE(g_compiler->type_def_u16);
		DECL_TYPE(g_compiler->type_def_u32);
		DECL_TYPE(g_compiler->type_def_u64);
		DECL_TYPE(g_compiler->type_def_f32);
		DECL_TYPE(g_compiler->type_def_f64);
	}

	auto tmp = this->lexer;
	this->lexer = new Lexer(abs_path, this->lexer);
	this->block(parent);

	this->all_lines += this->lexer->buffer->location.line;
	this->global_notes.clear();
	delete this->lexer;
	this->lexer = tmp;

	SET_PATH(last_path);
	return parent;
}

void Parser::add (Ast_Statement* stm, Ast_Block* block) {
	if (!block) block = this->current_block;

	if (this->global_notes.size()) {
		stm->notes.insert(stm->notes.end(),
			this->global_notes.begin(), this->global_notes.end());
	}

	block->list.push_back(stm);
	if (block->is_global) this->to_next(&stm);
	else {
		if (stm->stm_type == AST_STATEMENT_DECLARATION) {
			auto decl = static_cast<Ast_Declaration*>(stm);
			if (decl->is_constant()) this->to_next(&stm);
		}
	}
}

void Parser::block (Ast_Block* inner_block) {
	auto _tmp = this->current_block;
	this->current_block = inner_block;

	auto stm = this->statement();
	while (stm != NULL) {
		this->add(stm);

		while (this->pending_imports.size()) {
			auto import = this->pending_imports.front();
			this->pending_imports.pop_front();

			// @Incomplete: don't assume expression is a string
			auto literal = static_cast<Ast_Literal*>(import->target);
			this->run(literal->string_value, this->current_block);
		}

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
		} else report_error_stop(&note->location, "All notes must have a name");
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
	delete ident;

	auto other = this->current_block->find_non_const_declaration(decl->name);
	if (other && decl->scope == other->scope) {
		report_error(&decl->location, "re-declaration of variable '%s'", decl->name);
		report_error_stop(&other->location, "previous declaration here");
	}

	if (this->current_block->is_global) {
		decl->decl_flags |= AST_DECL_FLAG_GLOBAL;
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

Ast_Declaration* Parser::declaration_or_type () {
	auto decl = this->declaration();
	if (decl) return decl;
	else {
		decl = AST_NEW(Ast_Declaration);
		decl->type = this->type_instance();
		return decl;
	}
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
			call->fn = output;
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
					report_error_stop(&stm->location, "Only declarations can go inside a struct");
				}
			}
			delete _block;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		auto func = AST_NEW(Ast_Function);
		func->scope = AST_NEW(Ast_Block, this->current_block);
		func->scope->scope_of = func;

		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			auto _tmp = this->current_block;
			this->current_block = func->scope;

			auto decl = this->declaration();
			while (decl != NULL) {
				decl->decl_flags &= ~AST_DECL_FLAG_GLOBAL;
				func->arg_decls.push_back(decl);

				if (!this->lexer->optional_skip(TOKEN_COMMA)) break;
				decl = this->declaration();
			}
			this->lexer->check_skip(TOKEN_PAR_CLOSE);

			this->current_block = _tmp;
		}

		if (this->lexer->optional_skip(TOKEN_ARROW)) {
			func->ret_type = this->type_instance();
		} else func->ret_type = g_compiler->type_def_void;

		if (this->lexer->optional_skip(TOKEN_BRAC_OPEN)) {
			this->block(func->scope);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return func;
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
	} else if (this->lexer->optional_skip(TOKEN_FILE)) {
		return ast_make_literal(this->lexer->buffer->location.filename);
	} else if (this->lexer->optional_skip(TOKEN_LINE)) {
		return ast_make_literal(this->lexer->buffer->location.line);
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
		return g_compiler->types->get_or_create_pointer_type(base_type);
	} else if (this->lexer->optional_skip(TOKEN_SQ_BRAC_OPEN)) {
		auto length = this->expression();
		if (length) {
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();

			auto _array = AST_NEW(Ast_Array_Type);
			_array->length_exp = length;
			_array->base = base_type;
			return _array;
		} else {
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			auto base_type = this->type_instance();
			return g_compiler->types->get_or_create_slice_type(base_type);
		}
	} else {
		auto ident = this->ident();
		if (ident) {
			auto decl = this->current_block->find_const_declaration(ident->name);
			if (decl && decl->expression
					&& decl->expression->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
				delete ident;
				return decl->expression;
			} else return ident;
		} else return NULL;
	}
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);

	if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		Ast_Declaration* decl = this->declaration_or_type();
		while (decl != NULL) {
			decl->decl_flags &= ~AST_DECL_FLAG_GLOBAL;
			fn_type->arg_types.push_back(decl->type);

			if (!this->lexer->optional_skip(TOKEN_COMMA)) break;
			decl = this->declaration_or_type();
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}

	if (this->lexer->optional_skip(TOKEN_ARROW)) {
		fn_type->ret_type = this->type_instance();
	} else fn_type->ret_type = g_compiler->type_def_void;

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
