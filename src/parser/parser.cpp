#include "parser/parser.hpp"

#include "compiler.hpp"

template <typename T>
T* setup_ast_node (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}

#define AST_NEW(T, ...) setup_ast_node(lexer, new T(__VA_ARGS__))

Parser::Parser (Light_Compiler* compiler, const char* filepath) {
	this->compiler = compiler;
	this->lexer = new Lexer(filepath);
}

void push_new_type (Parser* parser, Ast_Block* block, const char* name, Ast_Type_Definition* type_def) {
	auto type_decl = ast_make_declaration(name, type_def);
	block->list.push_back(type_decl);
	parser->to_next(type_decl);
}

void push_new_type (Parser* parser, Ast_Block* block, Ast_Struct_Type* type_def) {
	push_new_type(parser, block, type_def->name, type_def);
}

Ast_Block* Parser::top_level_block () {
	auto _block = AST_NEW(Ast_Block);
	_block->is_global = true;

	push_new_type(this, _block, Light_Compiler::inst->type_def_type);
	push_new_type(this, _block, Light_Compiler::inst->type_def_void);
	push_new_type(this, _block, Light_Compiler::inst->type_def_bool);
	push_new_type(this, _block, Light_Compiler::inst->type_def_s8);
	push_new_type(this, _block, Light_Compiler::inst->type_def_s16);
	push_new_type(this, _block, Light_Compiler::inst->type_def_s32);
	push_new_type(this, _block, Light_Compiler::inst->type_def_s64);
	push_new_type(this, _block, Light_Compiler::inst->type_def_u8);
	push_new_type(this, _block, Light_Compiler::inst->type_def_u16);
	push_new_type(this, _block, Light_Compiler::inst->type_def_u32);
	push_new_type(this, _block, Light_Compiler::inst->type_def_u64);
	push_new_type(this, _block, Light_Compiler::inst->type_def_f32);
	push_new_type(this, _block, Light_Compiler::inst->type_def_f64);
	push_new_type(this, _block, "string", Light_Compiler::inst->type_def_string);

	this->block(_block);
	return _block;
}

void Parser::block (Ast_Block* insert_block) {
	auto _tmp = this->current_block;
	this->current_block = insert_block;

	Ast_Statement* stm;
	while (stm = this->statement()) {
		if (stm->stm_type == AST_STATEMENT_IMPORT) {
			auto imp = static_cast<Ast_Import*>(stm);

			this->lexer = this->lexer->push(imp->filepath);
			if (this->lexer->buffer->is_valid()) {
				this->block(this->current_block);
			} else this->compiler->error_stop(imp, "Can't open import file: '%s'", imp->filepath);
			this->lexer = this->lexer->pop();
		} else {
			this->current_block->list.push_back(stm);
			if (this->current_block->parent == NULL) {
				this->to_next(stm);
			} else {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					if (decl->is_constant()) this->to_next(stm);
				}
			}
		}

		if (this->lexer->isNextType(TOKEN_EOF)) break;
	}

	this->current_block = _tmp;
}

Ast_Note* Parser::note () {
	if (this->lexer->optional_skip(TOKEN_HASH)) {
		auto note = AST_NEW(Ast_Note);
		note->name = this->lexer->text();
		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			note->arguments = this->comma_separated_arguments();
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
		}
		return note;
	} else return NULL;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_EOF: this->lexer->report_unexpected();
		case TOKEN_STM_END: {
			this->lexer->skip(1);
			return NULL;
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
		case TOKEN_IMPORT: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Import);
			if (this->lexer->isNextType(TOKEN_STRING))
				output->filepath = this->lexer->text();
			else Light_Compiler::inst->error_stop(output, "Import statements must be followed by string literal!");
			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip(1);
			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return _block;
		}
		case TOKEN_IF: {
			if (this->lexer->optional_skip(TOKEN_IF)) {
				auto stm_if = AST_NEW(Ast_If);
				stm_if->condition = this->expression();
				stm_if->then_statement = this->statement();
				if (this->lexer->optional_skip(TOKEN_ELSE)) {
					stm_if->else_statement = this->statement();
				}
				return stm_if;
			} else return NULL;
		}
		case TOKEN_WHILE: {
			if (this->lexer->optional_skip(TOKEN_WHILE)) {
				auto stm_while = AST_NEW(Ast_While);
				stm_while->condition = this->expression();
				stm_while->statement = this->statement();
				return stm_while;
			} else return NULL;
		}
		case TOKEN_RETURN: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Return);
			output->exp = this->expression();
			output->block = this->current_block;
			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		default: {
			auto ident = this->ident();
			if (this->lexer->isNextType(TOKEN_COLON)) {
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
				if (!_struct->name) _struct->name = strdup(decl->name);
			}
		}
	}

	this->lexer->optional_skip(TOKEN_STM_END);

	return decl;
}

Ast_Expression* Parser::expression (Ast_Ident* initial, short minPrecedence) {
	Ast_Expression* output;
    if (output = this->_atom(initial)) {
        Token_Type tt = this->lexer->nextType;
		auto precedence = Ast_Binary::getPrecedence(tt);
		while (precedence >= minPrecedence) {
			this->lexer->skip(1);

			int nextMinPrec = precedence;
			if (Ast_Binary::getLeftAssociativity(tt))
				nextMinPrec += 1;

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, nextMinPrec);
			_tmp->lhs = output;
			output = _tmp;

			if (tt == TOKEN_SQ_BRAC_OPEN) this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);

			tt = this->lexer->nextType;
			precedence = Ast_Binary::getPrecedence(tt);
		}
    }
	return output;
}

Ast_Comma_Separated_Arguments* Parser::comma_separated_arguments (Ast_Expression* exp) {
	if (!exp) exp = this->expression();
	if (!exp) return NULL;

	auto arguments = AST_NEW(Ast_Comma_Separated_Arguments);
	while (exp != NULL) {
		arguments->values.push_back(exp);
		this->lexer->optional_skip(TOKEN_COMMA);
		exp = this->expression();
	}
	return arguments;
}

Ast_Function* Parser::function (Ast_Function_Type* fn_type) {
	if (this->lexer->optional_skip(TOKEN_BRAC_OPEN)) {
		auto fn = AST_NEW(Ast_Function);
		fn->type = fn_type;
		fn->scope = AST_NEW(Ast_Block, this->current_block);
		fn->scope->scope_of = fn;
		this->block(fn->scope);
		this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		return fn;
	} else return NULL;
}

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->lexer->isNextType(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
			return this->call(output);
		} else return output;
	} else if (this->lexer->optional_skip(TOKEN_STRUCT)) {
		auto _struct = AST_NEW(Ast_Struct_Type);

		if (this->lexer->isNextType(TOKEN_ID))
			_struct->name = this->lexer->text();

		if (this->lexer->isNextType(TOKEN_BRAC_OPEN)) {
			this->lexer->skip(1);

			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			for (auto stm : _block->list) {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					decl->_struct = _struct;
					_struct->attributes.push_back(decl);
				} else {
					Light_Compiler::inst->error_stop(stm, "Only declarations can go inside a struct!");
				}
			}
			delete _block;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return _struct;
	} else if (this->lexer->optional_skip(TOKEN_CAST)) {
		auto cast = AST_NEW(Ast_Cast);
		this->lexer->check_skip(TOKEN_PAR_OPEN);
		cast->cast_to = this->_atom();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		cast->value = this->_atom();
		return cast;
	} else if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		return this->function(this->function_type());
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		return AST_NEW(Ast_Pointer, this->_atom());
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
		auto array = AST_NEW(Ast_Array_Type);
		array->count = this->_atom();
		this->lexer->check_skip(TOKEN_SQ_BRAC_CLOSE);
		array->base = this->type_definition();
		return array;
	} else if (this->lexer->optional_skip(TOKEN_MUL)) {
		auto ptr = AST_NEW(Ast_Pointer_Type);
		ptr->base = this->type_definition();
		return ptr;
	} else if (this->lexer->optional_skip(TOKEN_FUNCTION)) {
		return this->function_type();
	} else return this->ident();
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);

	if (this->lexer->optional_skip(TOKEN_PAR_OPEN)) {
		Ast_Declaration* decl;
		while (decl = this->declaration()) {
			decl->decl_flags &= ~AST_DECL_FLAG_GLOBAL;
			fn_type->parameter_decls.push_back(decl);

			if (!this->lexer->optional_skip(TOKEN_COMMA)) break;
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}

	if (this->lexer->optional_skip(TOKEN_ARROW)) {
		fn_type->return_type = this->type_definition();
	} else fn_type->return_type = Light_Compiler::inst->type_def_void;

	return fn_type;
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->lexer->nextType) {
		case TOKEN_STRING: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_STRING;
			output->string_value = this->lexer->text();
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			auto number_str = this->lexer->text();
			if (strstr(number_str, ".") != NULL) {
				output->literal_type = AST_LITERAL_DECIMAL;
				output->decimal_value = atof(number_str);
			} else if (strstr(number_str, "x") != NULL) {
				output->literal_type = AST_LITERAL_UNSIGNED_INT;
				output->uint_value = strtoull(number_str, NULL, 16);
			}else {
				output->literal_type = AST_LITERAL_UNSIGNED_INT;
				output->uint_value = strtoull(number_str, NULL, 10);
			}
			break;
		}
		default: break;
	}
	return output;
}

Ast_Function_Call* Parser::call (Ast_Expression* callee) {
	auto output = AST_NEW(Ast_Function_Call);
	output->fn = callee;
	output->args = this->comma_separated_arguments();
	this->lexer->check_skip(TOKEN_PAR_CLOSE);
	return output;
}

Ast_Ident* Parser::ident (const char* name) {
	if (!name && !this->lexer->isNextType(TOKEN_ID)) return NULL;

	auto output = AST_NEW(Ast_Ident, this->current_block);
	output->name = name ? name : this->lexer->text();
	return output;
}
