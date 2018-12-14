#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

#include "pipeline/pipes/type_checking.hpp"

#define TMP_NAME_SIZE 20

struct Cast_Arrays : Scoped_Statement_Pipe {
	PIPE_NAME(Cast_Arrays)

	size_t tmp_var_count = 0;

    void handle (Ast_Cast** cast_ptr) {
        auto cast = (*cast_ptr);

        if (cast->is_array_to_slice_cast) {
            auto stm_location = this->get_current_stm_location();
            if (stm_location != this->current_scope->statements.end()) {
                auto type_checker = new Type_Checking();

                auto slice_type = static_cast<Ast_Slice_Type*>(cast->cast_to);
                auto base_type = slice_type->get_typed_base();

				char tmp_name[TMP_NAME_SIZE];
				sprintf_s(tmp_name, TMP_NAME_SIZE, "$tmp$slice$%zd", tmp_var_count++);

                // $tmp : []Type;
                auto slice_declaration = ast_make_declaration_with_type(tmp_name, slice_type);
                slice_declaration->scope = this->current_scope;
                type_checker->handle(&slice_declaration);

                // $tmp.length = array.length;
                auto tmp_ident1 = ast_make_ident(tmp_name);
                tmp_ident1->declaration = slice_declaration;
                tmp_ident1->inferred_type = slice_type;

                auto tmp_length_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident1, ast_make_ident("length"));
                auto arr_length_property = ast_make_binary(AST_BINARY_ATTRIBUTE, cast->value, ast_make_ident("length"));
                auto assign_length = ast_make_binary(AST_BINARY_ASSIGN, tmp_length_property, arr_length_property);
                tmp_length_property->inferred_type = Types::type_u64;
                arr_length_property->inferred_type = Types::type_u64;
                assign_length->inferred_type = Types::type_u64;
                type_checker->handle(&assign_length);

                // $tmp.data = array.data;
                auto tmp_ident2 = ast_make_ident(tmp_name);
                tmp_ident2->declaration = slice_declaration;
                tmp_ident2->inferred_type = slice_type;

                auto tmp_data_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident2, ast_make_ident("data"));
                auto arr_data_property = ast_make_binary(AST_BINARY_ATTRIBUTE, cast->value, ast_make_ident("data"));
                auto assign_data = ast_make_binary(AST_BINARY_ASSIGN, tmp_data_property, arr_data_property);
                tmp_data_property->inferred_type = base_type;
                arr_data_property->inferred_type = base_type;
                assign_data->inferred_type = base_type;
                type_checker->handle(&assign_data);

                stm_location = this->current_scope->statements.insert(stm_location, assign_data);
                stm_location = this->current_scope->statements.insert(stm_location, assign_length);
                stm_location = this->current_scope->statements.insert(stm_location, slice_declaration);

                auto tmp_ident3 = ast_make_ident(tmp_name);
                tmp_ident3->declaration = slice_declaration;
                tmp_ident3->inferred_type = slice_type;
                *cast_ptr = reinterpret_cast<Ast_Cast*>(tmp_ident3);

                delete type_checker;
            }
        }
    }
};
