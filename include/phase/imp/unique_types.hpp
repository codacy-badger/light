#pragma once

#include "phase/phase.hpp"
#include "phase/ast_ref_navigator.hpp"

#include "ast/type_table.hpp"
#include "ast/ast_cloner.hpp"
#include "compiler_events.hpp"

struct Unique_Types : Phase, Ast_Ref_Navigator {
    Type_Table* type_table;

    size_t uniqued_types = 0;

    Unique_Types(Type_Table* type_table) : Phase("Unique Types", CE_MODULE_UNIQUE_TYPES, true) {
        this->type_table = type_table;
    }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Ref_Navigator::ast_handle(&global_scope);

        this->push(global_scope);
    }

    void unique (Ast_Type** type_ptr) {
        if (!this->type_table->is_unique(*type_ptr)) {
            auto old_ptr = (*type_ptr);

            (*type_ptr) = this->type_table->find_unique(*type_ptr);
            Types::set_name(*type_ptr);

            if (old_ptr != (*type_ptr)) {
                this->uniqued_types += 1;
            }
        }
    }

    void ast_handle (Ast_Expression** exp_ptr) {
        Ast_Ref_Navigator::ast_handle(exp_ptr);
        this->ast_handle(&(*exp_ptr)->inferred_type);
    }

    void ast_handle (Ast_Type** type_ptr) {
        Ast_Ref_Navigator::ast_handle(type_ptr);
        this->unique(type_ptr);
    }

    /*void ast_handle (Ast_Binary** binary_ptr) {
        // @Info rhs of attribute access can't have type,
        // so we can't unique them
        if ((*binary_ptr)->binary_op != AST_BINARY_ATTRIBUTE) {
            Ast_Ref_Navigator::ast_handle(binary_ptr);
        }
    }*/

	void print_extra_metrics() {
		print_extra_metric("Uniqued types", "%zd", this->uniqued_types);
	}
};
