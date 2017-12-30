#include "parser/parser.hpp"

#include <errno.h>

#include "compiler.hpp"

template <typename T>
T* setup_ast_node (Lexer* lexer, T* node) {
	node->location = lexer->buffer->location;
	return node;
}

#define AST_NEW(T, ...) setup_ast_node(lexer, new T(__VA_ARGS__))

FILE* open_file_or_stop (const char* filename, Location* location = NULL) {
	FILE* file_ptr = NULL;
	auto err = fopen_s(&file_ptr, filename, "r");
	if (err) {
		char buf[256];
		strerror_s(buf, sizeof buf, err);
		report_error_stop(location, "Cannot open file '%s': %s", filename, buf);
	}
	return file_ptr;
}

Parser::Parser (const char* filepath) {
	auto file = open_file_or_stop(filepath);
	this->lexer = new Lexer(file, filepath);
}

void push_new_type (Parser* parser, Ast_Block* block, Ast_Struct_Type* type_def) {
	auto type_decl = ast_make_declaration(type_def->name, type_def);
	block->list.push_back(type_decl);
	parser->to_next(type_decl);
}

Ast_Block* Parser::top_level_block () {
	auto _block = AST_NEW(Ast_Block);
	_block->is_global = true;

	push_new_type(this, _block, g_compiler->type_def_type);
	push_new_type(this, _block, g_compiler->type_def_void);
	push_new_type(this, _block, g_compiler->type_def_bool);
	push_new_type(this, _block, g_compiler->type_def_s8);
	push_new_type(this, _block, g_compiler->type_def_s16);
	push_new_type(this, _block, g_compiler->type_def_s32);
	push_new_type(this, _block, g_compiler->type_def_s64);
	push_new_type(this, _block, g_compiler->type_def_u8);
	push_new_type(this, _block, g_compiler->type_def_u16);
	push_new_type(this, _block, g_compiler->type_def_u32);
	push_new_type(this, _block, g_compiler->type_def_u64);
	push_new_type(this, _block, g_compiler->type_def_f32);
	push_new_type(this, _block, g_compiler->type_def_f64);
	push_new_type(this, _block, g_compiler->type_def_usize);

	this->block(_block);
	return _block;
}

void Parser::block (Ast_Block* inner_block) {
	auto _tmp = this->current_block;
	this->current_block = inner_block;

	Ast_Statement* stm = this->statement();
	while (stm != NULL) {
		this->current_block->list.push_back(stm);
		if (this->current_block->is_global) {
			this->to_next(stm);
		} else {
			if (stm->stm_type == AST_STATEMENT_DECLARATION) {
				auto decl = static_cast<Ast_Declaration*>(stm);
				if (decl->is_constant()) this->to_next(stm);
			}
		}

		if (this->lexer->is_next_type(TOKEN_EOF)) break;
		else stm = this->statement();
	}

	this->current_block = _tmp;
}

Ast_Note* Parser::note () {
	if (this->lexer->optional_skip(TOKEN_HASH)) {
		auto note = AST_NEW(Ast_Note);
		note->name = this->lexer->text();
		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			this->comma_separated_arguments(&note->arguments);
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
		}
		return note;
	} else return NULL;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->next_type) {
		case TOKEN_EOF: this->lexer->report_unexpected();
		case TOKEN_STM_END: {
			this->lexer->skip();
			return this->statement();
		}
		case TOKEN_IMPORT: {
			this->lexer->skip();

			if (this->lexer->is_next_type(TOKEN_STRING)) {
				auto filepath = this->lexer->text();
				auto file = open_file_or_stop(filepath, &this->lexer->buffer->location);

				auto tmp = this->lexer;
				this->lexer = new Lexer(file, filepath);
				this->block(this->current_block);
				this->lexer = tmp;

				fclose(file);
			} else report_error_stop(&this->lexer->buffer->location, "Import statements must be followed by string literal");
			this->lexer->optional_skip(TOKEN_STM_END);

			return this->statement();
		}
		case TOKEN_HASH: {
			vector<Ast_Note*> notes;
			auto note = this->note();
			while (note != NULL) {
				notes.push_back(note);
				note = this->note();
			}

			auto stm = this->statement();
			stm->notes = notes;
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

	if (this->current_block->is_global) {
		decl->decl_flags |= AST_DECL_FLAG_GLOBAL;
	}

	if (this->lexer->check_skip(TOKEN_COLON)) {
		decl->type = this->type_definition();
	}

	if (this->lexer->optional_skip(TOKEN_COLON)) {
		decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
	} else this->lexer->optional_skip(TOKEN_EQUAL);

	decl->expression = this->expression();

	if (decl->expression && decl->is_constant()) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			auto fn = static_cast<Ast_Function*>(decl->expression);
			if (!fn->name) fn->name = decl->name;
		} else if (decl->expression->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->expression);
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
		auto precedence = Ast_Binary::getPrecedence(tt);
		while (precedence >= min_precedence) {
			this->lexer->skip();

			short nextMinPrec = precedence;
			if (Ast_Binary::getLeftAssociativity(tt)) nextMinPrec += 1;

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, nextMinPrec);
			_tmp->lhs = output;
			output = _tmp;

			if (tt == TOKEN_SQ_BRAC_OPEN) this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);

			tt = this->lexer->next_type;
			precedence = Ast_Binary::getPrecedence(tt);
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
					report_error_stop(&stm->location, "Only declarations can go inside a struct!");
				}
			}
			delete _block;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		auto fn_type = this->function_type();
		if (this->lexer->optional_skip(TOKEN_BRAC_OPEN)) {
			auto func = AST_NEW(Ast_Function);
			func->type = fn_type;
			func->scope = AST_NEW(Ast_Block, this->current_block);
			func->scope->scope_of = func;
			this->block(func->scope);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return func;
		} else return fn_type;
	} else if (this->lexer->optional_skip(TOKEN_CAST)) {
		auto cast = AST_NEW(Ast_Cast);
		this->lexer->check_skip(TOKEN_PAR_OPEN);
		cast->cast_to = this->type_definition();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		cast->value = this->_atom();
		return cast;
	} else if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		return AST_NEW(Ast_Unary, TOKEN_MUL, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_EXCLAMATION)) {
		return AST_NEW(Ast_Unary, TOKEN_EXCLAMATION, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_SUB)) {
		return AST_NEW(Ast_Unary, TOKEN_SUB, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_AMP)) {
		return AST_NEW(Ast_Unary, TOKEN_AMP, this->_atom());
	} else if (this->lexer->optional_skip(TOKEN_ADD)) {
		return this->expression();
	} else return this->literal();
}

Ast_Expression* Parser::type_definition () {
	if (this->lexer->optional_skip(TOKEN_SQ_BRAC_OPEN)) {
		auto count = this->_atom();
		if (count) {
			auto _array = AST_NEW(Ast_Array_Type);
			_array->count = count;
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			_array->base = this->type_definition();
			return _array;
		} else {
			this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
			return ast_make_slice_type(this->type_definition());
		}
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		return AST_NEW(Ast_Pointer_Type, this->type_definition());
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		return this->function_type();
	} else {
		auto ident = this->ident();
		if (ident != NULL) {
			auto _struct = g_compiler->types->get_struct_type(ident->name);
			if (_struct == NULL) return ident;
			else {
				delete ident;
				return _struct;
			}
		} else return NULL;
	}
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);

	if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		Ast_Declaration* decl = this->declaration();
		while (decl != NULL) {
			decl->decl_flags &= ~AST_DECL_FLAG_GLOBAL;
			fn_type->parameter_decls.push_back(decl);

			if (!this->lexer->optional_skip(TOKEN_COMMA)) break;
			decl = this->declaration();
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}

	if (this->lexer->optional_skip(TOKEN_ARROW)) {
		fn_type->return_type = this->type_definition();
	} else fn_type->return_type = g_compiler->type_def_void;

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

	// this is the right time to do this, since on a non-is_constant reference
	// the declaration should already be in the scope, even if it's still not resolved.
	ident->declaration = this->current_block->find_non_const_declaration(ident->name);
	return ident;
}
