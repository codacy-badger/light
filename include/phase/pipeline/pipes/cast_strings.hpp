#pragma once

#include "phase/pipeline/scoped_statement_pipe.hpp"

#define TMP_NAME_SIZE 20

struct Cast_Strings : Scoped_Statement_Pipe {
	Cast_Strings () { this->pipe_name = "Cast_Strings"; }

	size_t tmp_var_count = 0;

    void handle (Ast_Literal** lit_ptr) {
        auto lit = (*lit_ptr);

        if (lit->literal_type == AST_LITERAL_STRING) {
            auto stm_location = this->get_current_stm_location();
            if (stm_location != this->current_scope->statements.end()) {
                auto len_literal = ast_make_literal(strlen(lit->string_value));
                auto ptr_literal = ast_make_literal((uint64_t) lit->string_value);

				char tmp_name[TMP_NAME_SIZE];
				sprintf_s(tmp_name, TMP_NAME_SIZE, "$tmp$string$%zd", tmp_var_count++);

                // $tmp : []Type;
                auto string_declaration = ast_make_declaration_with_type(tmp_name, Types::type_string);
                string_declaration->scope = this->current_scope;
				string_declaration->location = lit->location;

                // $tmp.length = array.length;
                auto tmp_ident1 = ast_make_ident(tmp_name);
                tmp_ident1->declaration = string_declaration;
                tmp_ident1->inferred_type = Types::type_string;
				tmp_ident1->location = lit->location;

                auto tmp_length_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident1, ast_make_ident("length"));
                auto assign_length = ast_make_binary(AST_BINARY_ASSIGN, tmp_length_property, len_literal);
				tmp_length_property->inferred_type = Types::type_u64;
				len_literal->inferred_type = Types::type_u64;
				tmp_length_property->location = lit->location;
				len_literal->location = lit->location;

                // $tmp.data = array.data;
                auto tmp_ident2 = ast_make_ident(tmp_name);
                tmp_ident2->declaration = string_declaration;
                tmp_ident2->inferred_type = Types::type_string;
				tmp_ident2->location = lit->location;

                auto tmp_data_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident2, ast_make_ident("data"));
                auto assign_data = ast_make_binary(AST_BINARY_ASSIGN, tmp_data_property, ptr_literal);
				tmp_data_property->inferred_type = Types::type_u64;
				ptr_literal->inferred_type = Types::type_u64;
				tmp_data_property->location = lit->location;
				ptr_literal->location = lit->location;

                stm_location = this->current_scope->statements.insert(stm_location, assign_data);
                stm_location = this->current_scope->statements.insert(stm_location, assign_length);
                stm_location = this->current_scope->statements.insert(stm_location, string_declaration);

                auto tmp_ident3 = ast_make_ident(tmp_name);
                tmp_ident3->declaration = string_declaration;
                tmp_ident3->inferred_type = Types::type_string;
				tmp_ident3->location = lit->location;
                *lit_ptr = reinterpret_cast<Ast_Literal*>(tmp_ident3);
            } else Logger::internal(lit, "Current statement can't be found");
        }
    }
};
