#pragma once

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

Ast_Block* Parser::top_level_block () {
	auto _block = AST_NEW(Ast_Block, this->current_block);

	auto type_decl = ast_make_declaration("type", Light_Compiler::instance->type_def_type);
	_block->list.push_back(type_decl);
	this->to_next(type_decl);

	auto void_decl = ast_make_declaration("void", Light_Compiler::instance->type_def_void);
	_block->list.push_back(void_decl);
	this->to_next(void_decl);

	auto i32_decl = ast_make_declaration("i32", Light_Compiler::instance->type_def_i32);
	_block->list.push_back(i32_decl);
	this->to_next(i32_decl);

	this->block(_block);
	return _block;
}

void Parser::block (Ast_Block* insert_block) {
	this->current_block = insert_block;
	this->on_block_begin(this->current_block);

	Ast_Statement* stm;
	while (stm = this->statement()) {
		if (stm->stm_type == AST_STATEMENT_IMPORT) {
			auto imp = static_cast<Ast_Import*>(stm);
			if (imp->import_flags & IMPORT_INCLUDE_CONTENT) {
				this->lexer = this->lexer->push(imp->filepath);
				if (this->lexer->buffer->is_valid()) {
					auto include_block = AST_NEW(Ast_Block, this->current_block);
					this->block(include_block);
					this->current_block->list.insert(this->current_block->list.end(),
						include_block->list.begin(), include_block->list.end());
					delete include_block;
				} else this->compiler->error(imp,
					"Can't open import file: '%s'", imp->filepath);
				this->lexer = this->lexer->pop();
			}
		} else {
			this->current_block->list.push_back(stm);
			if (this->current_block->parent == NULL) {
				this->to_next(stm);
			} else {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					if (decl->decl_flags & DECL_FLAG_CONSTANT)
						this->to_next(stm);
				}
			}
		}

		if (this->lexer->isNextType(TOKEN_EOF)) break;
	}

	this->on_block_end(this->current_block);
	this->current_block = this->current_block->parent;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_EOF: this->lexer->report_unexpected();
		case TOKEN_STM_END: {
			this->lexer->skip(1);
			return NULL;
		}
		case TOKEN_RETURN: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Return);
			output->exp = this->expression();
			output->block = this->current_block;
			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_IMPORT: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Import);

			if (this->lexer->optional_skip(TOKEN_EXCLAMATION))
				output->import_flags |= IMPORT_INCLUDE_CONTENT;
			else if (this->lexer->optional_skip(TOKEN_AT))
				output->import_flags |= IMPORT_IS_NATIVE;

			if (this->lexer->isNextType(TOKEN_STRING))
				output->filepath = this->lexer->text();

			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_HASH: {
			this->lexer->skip(1);
			auto note = AST_NEW(Ast_Note);
			note->name = this->lexer->text();
			auto next_stm = this->statement();
			next_stm->notes.push_back(note);
			return next_stm;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip(1);
			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return _block;
		}
		case TOKEN_ID: {
			auto ident = this->ident();
			if (this->lexer->isNextType(TOKEN_COLON)) {
				return this->declaration(ident);
			} else {
				auto exp = this->expression(ident);
				if (exp) {
					if (this->lexer->isNextType(TOKEN_STM_END)) {
						this->lexer->skip(1);
						return exp;
					} else {
						auto output = AST_NEW(Ast_Return);
						output->exp = exp;
						return output;
					}
				} else return NULL;
			}
		}
		default: {
			auto exp = this->expression();
			if (exp) {
				if (this->lexer->isNextType(TOKEN_STM_END)) {
					this->lexer->skip(1);
					return exp;
				} else {
					auto output = AST_NEW(Ast_Return);
					output->exp = exp;
					return output;
				}
			} else return NULL;
		}
	}
}

Ast_Declaration* Parser::declaration (Ast_Ident* ident) {
	if (!ident) {
		if (this->lexer->isNextType(TOKEN_ID))
			ident = this->ident();
		else return NULL;
	}

	auto decl = AST_NEW(Ast_Declaration);
	decl->name = ident->name;
	delete ident;

	if (this->lexer->isNextType(TOKEN_COLON)) {
		this->lexer->skip(1);
		decl->type = this->_atom();
	}

	if (this->lexer->isNextType(TOKEN_COLON)) {
		this->lexer->skip(1);
		decl->decl_flags |= DECL_FLAG_CONSTANT;
	} else if (this->lexer->isNextType(TOKEN_EQUAL)) {
		this->lexer->skip(1);
	}

	decl->expression = this->expression();

	if (decl->expression && decl->decl_flags & DECL_FLAG_CONSTANT) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			auto fn = static_cast<Ast_Function*>(decl->expression);
			if (!fn->name) fn->name = decl->name;
		} else if (decl->expression->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->expression);
			if (defn_ty->typedef_type == AST_TYPEDEF_STRUCT) {
				auto _struct = static_cast<Ast_Struct_Type*>(defn_ty);
				if (!_struct->name) _struct->name = decl->name;
			}
		}
	}

	if (this->lexer->isNextType(TOKEN_STM_END))
		this->lexer->skip(1);

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

			tt = this->lexer->nextType;
			precedence = Ast_Binary::getPrecedence(tt);
		}
    }
	return output;
}

Ast_Function* Parser::function (Ast_Function_Type* fn_type) {
	if (this->lexer->isNextType(TOKEN_BRAC_OPEN)) {
		this->lexer->skip(1);
		auto fn = AST_NEW(Ast_Function);
		fn->type = fn_type;
		fn->scope = AST_NEW(Ast_Block, this->current_block);
		fn->scope->scope_of = fn;
		this->block(fn->scope);
		this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		return fn;
	} else return NULL;
}

Ast_Function_Type* Parser::function_type () {
	auto fn_type = AST_NEW(Ast_Function_Type);
	fn_type->inferred_type = Light_Compiler::instance->type_def_type;

	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		Ast_Declaration* decl;
		while (decl = this->declaration()) {
			fn_type->parameter_types.push_back(decl->type);
			decl = this->declaration();
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}
	if (this->lexer->isNextType(TOKEN_ARROW)) {
		this->lexer->skip(1);
		fn_type->return_type = this->expression();
	} else fn_type->return_type = this->ident("void");

	return fn_type;
}

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->lexer->isNextType(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			return this->call(output);
		} else return output;
	} else if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->isNextType(TOKEN_FUNCTION)) {
		this->lexer->skip(1);

		auto fn_type = this->function_type();
		if (this->lexer->isNextType(TOKEN_BRAC_OPEN)) {
			return this->function(fn_type);
		} else return fn_type;

	} else if (this->lexer->isNextType(TOKEN_STRUCT)) {
		this->lexer->skip(1);

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
					_struct->attributes.push_back(decl);
				} else {
					Light_Compiler::instance->error_stop(stm, "Only declarations can go inside a struct!");
				}
			}
			delete _block;
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}


		return _struct;
	} else if (this->lexer->isNextType(TOKEN_SUB)) {
		this->lexer->skip(1);
		auto unop = AST_NEW(Ast_Unary, TOKEN_SUB);
		unop->exp = this->_atom();
		return unop;
	} else if (this->lexer->isNextType(TOKEN_ADD)) {
		this->lexer->skip(1);
		return this->expression();
	} else return this->literal();
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
			output->literal_type = AST_LITERAL_INTEGER;
			output->integer_value = atoi(this->lexer->text());
			break;
		}
		default: break;
	}
	return output;
}

Ast_Function_Call* Parser::call (Ast_Expression* callee) {
	Ast_Function_Call* output = NULL;
	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);

		output = AST_NEW(Ast_Function_Call);
		output->fn = callee;

		Ast_Expression* exp = NULL;
		while (exp = this->expression()) {
			output->parameters.push_back(exp);
			if (this->lexer->isNextType(TOKEN_COMMA))
				this->lexer->skip(1);
		}

		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}
	return output;
}

Ast_Ident* Parser::ident (const char* name) {
	if (name) {
		Ast_Ident* output = AST_NEW(Ast_Ident);
		output->name = name;
		output->declaration = this->current_block->find_declaration(name);
		return output;
	} else if (this->lexer->isNextType(TOKEN_ID)) {
		Ast_Ident* output = AST_NEW(Ast_Ident);
		output->name = this->lexer->text();
		output->declaration = this->current_block->find_declaration(output->name);
		return output;
	} else return NULL;
}
