#pragma once

#include "pipeline/scoped_statement_pipe.hpp"

struct Cast_Anys : Scoped_Statement_Pipe {
    Cast_Anys () { this->pipe_name = "Cast_Anys"; }

    size_t tmp_var_count = 0;

    void handle (Ast_Cast** cast_ptr) {
        auto cast = (*cast_ptr);

        if (cast->is_value_to_any_cast) {
            auto stm_location = this->get_current_stm_location();
            if (stm_location != this->current_scope->statements.end()) {
                // TODO: remove this! assign values manually
                auto type_checker = new Type_Checking();

                auto type = static_cast<Ast_Type_Instance*>(cast->value->inferred_type);

				char tmp_name[TMP_NAME_SIZE];
				sprintf_s(tmp_name, TMP_NAME_SIZE, "$tmp$any$%zd", tmp_var_count++);

                // $tmp : any;
                auto any_declaration = ast_make_declaration_with_type(tmp_name, Types::type_any);
                any_declaration->scope = this->current_scope;
                type_checker->handle(&any_declaration);

                // $tmp.type = type_id(value);
                auto tmp_ident1 = ast_make_ident(tmp_name);
                tmp_ident1->declaration = any_declaration;
                tmp_ident1->inferred_type = Types::type_any;

                // TODO: replace this by the proper type codes
                uint64_t type_code = 1;
                if (type != Types::type_u8) {
                    type_code = 2;
                }

                auto tmp_type_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident1, ast_make_ident("type"));
                auto assign_type = ast_make_binary(AST_BINARY_ASSIGN, tmp_type_property, ast_make_literal(type_code));
                tmp_type_property->inferred_type = Types::type_u64;
                assign_type->inferred_type = Types::type_u64;
                type_checker->handle(&assign_type);

                // $tmp.value = *value;
                auto tmp_ident2 = ast_make_ident(tmp_name);
                tmp_ident2->declaration = any_declaration;
                tmp_ident2->inferred_type = Types::type_any;

                auto tmp_value_property = ast_make_binary(AST_BINARY_ATTRIBUTE, tmp_ident2, ast_make_ident("value"));
                auto value_property = ast_make_unary(AST_UNARY_REFERENCE, cast->value);
                auto assign_value = ast_make_binary(AST_BINARY_ASSIGN, tmp_value_property, value_property);
                type_checker->handle(&assign_value);

                stm_location = this->current_scope->statements.insert(stm_location, assign_type);
                stm_location = this->current_scope->statements.insert(stm_location, assign_value);
                stm_location = this->current_scope->statements.insert(stm_location, any_declaration);

                auto tmp_ident3 = ast_make_ident(tmp_name);
                tmp_ident3->declaration = any_declaration;
                tmp_ident3->inferred_type = Types::type_any;
                *cast_ptr = reinterpret_cast<Ast_Cast*>(tmp_ident3);

                delete type_checker;
            }
        }
    }
};
