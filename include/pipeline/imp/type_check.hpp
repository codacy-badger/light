#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include "pipeline/service/type_inferrer.hpp"
#include "pipeline/service/type_table.hpp"
#include "pipeline/service/type_caster.hpp"

struct Type_Check : Compiler_Pipe<Ast_Statement*>, Ast_Ref_Navigator {
    Type_Inferrer* inferrer = NULL;
    Type_Table* type_table = NULL;
    Type_Caster* caster = NULL;

    Type_Check () : Compiler_Pipe("Type Check") { /* empty */ }

    void init () {
        this->inferrer = this->context->type_inferrer;
        this->type_table = this->context->type_table;
        this->caster = this->context->type_caster;
    }

    void handle (Ast_Statement* global_statement) {
        Ast_Ref_Navigator::ast_handle(&global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Scope* scope) {
        String_Map<std::vector<Ast_Declaration*>> decl_map;
        scope->find_all_declarations(&decl_map);

        for (auto entry : decl_map) {
            if (entry.second.size() > 1) {
                this->context->error(entry.second[0], "Multiple declarations with same name found for '%s':", entry.first);
                for (size_t i = 1; i < entry.second.size(); i++) {
                    this->context->error(entry.second[i], "Re-declaration of '%s' here", entry.first);
                }
                this->context->shutdown();
                return;
            }
        }

        Ast_Ref_Navigator::ast_handle(scope);
    }

    void ast_handle (Ast_Assign* assign) {
        Ast_Ref_Navigator::ast_handle(assign);

        auto success = this->caster->try_implicid_cast(assign->value->inferred_type,
            assign->variable->inferred_type, &assign->value);
        if (!success) {
            this->context->error(assign->value, "Assignment value cannot be implicitly casted from '%s' to '%s'",
                assign->value->inferred_type->name, assign->variable->inferred_type->name);
            this->context->shutdown();
            return;
        }
    }

    void ast_handle (Ast_Return* ret) {
        auto func = ret->scope->get_parent_function();
        if (!func) {
            this->context->error(ret, "Return statement found outside function scope");
            this->context->shutdown();
            return;
        }

        this->inferrer->infer(func);
        assert(func->inferred_type);
        assert(func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);

        auto func_type = static_cast<Ast_Function_Type*>(func->inferred_type);
        assert(func_type->ret_type->exp_type == AST_EXPRESSION_TYPE);
        auto typed_ret_type = static_cast<Ast_Type*>(func_type->ret_type);

        this->tuple_try_cast_subtypes(typed_ret_type, ret->expression);
        Ast_Ref_Navigator::ast_handle(ret);

        auto success = this->caster->try_implicid_cast(ret->expression->inferred_type,
            typed_ret_type, &ret->expression);
        if (!success) {
            this->context->error(ret, "Return value cannot be implicitly casted from '%s' to '%s'",
                ret->expression->inferred_type->name, typed_ret_type->name);
            this->context->shutdown();
            return;
        }
    }

    void ast_handle (Ast_Declaration* decl) {
        if (!decl->type && !decl->value) {
            this->context->error(decl, "Declarations must either have a value or a type");
            this->context->shutdown();
            return;
        }

        if (decl->value) {
            if (decl->type) {
                this->ast_handle(&decl->type);
                assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
                auto decl_type = static_cast<Ast_Type*>(decl->type);

                this->tuple_try_cast_subtypes(decl_type, decl->value);
                Ast_Ref_Navigator::ast_handle(decl);

                auto success = this->caster->try_implicid_cast(decl->value->inferred_type,
                    decl_type, &decl->value);
                if (!success) {
                    this->context->error(decl->value, "Expression cannot be implicitly casted from '%s' to '%s'",
                        decl->value->inferred_type->name, decl_type->name);
                    this->context->shutdown();
                    return;
                }
            } else {
                this->ast_handle(&decl->value);
                decl->type = decl->value->inferred_type;
            }
        } else this->ast_handle(&decl->type);

        assert(decl->type);
        assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
    }

    void ast_handle (Ast_Expression** exp_ptr) {
        Ast_Ref_Navigator::ast_handle(exp_ptr);
        this->inferrer->infer(*exp_ptr);
    }

    void ast_handle (Ast_Function_Call** call_ptr) {
        auto call = (*call_ptr);

        Ast_Ref_Navigator::ast_handle(call_ptr);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        if (call->func->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(call->func);

            auto arg_count = func_type->arg_types.size();

            if (call->arguments->unnamed.size() > arg_count) {
                this->context->error(call, "Too many arguments for function call, should have at most %zd", arg_count);
                this->context->shutdown();
                return;
            }

            call->arguments->unnamed.reserve(arg_count);
            for (size_t i = call->arguments->unnamed.size(); i < arg_count; i++) {
                call->arguments->unnamed.push_back(NULL);
            }

            for (auto entry : call->arguments->named) {
                auto arg_index = func->get_arg_index(entry.first);
                if (arg_index != INVALID_ARG_INDEX) {
                    auto existing_value = call->arguments->get_unnamed_value(arg_index);
                    if (existing_value) {
                        this->context->error(call, "Multiple values provided for argument '%s'", entry.first);
                        this->context->shutdown();
                        return;
                    } else {
                        call->arguments->unnamed[arg_index] = entry.second;
                    }
                } else {
                    //
                    // @Bug @TODO this could produce incorrect error messages, since
                    // function types contain the argument names, but are also uniqued!
                    // possible solution: the argument names should be stored in the Ast_Function
                    // nodes, but we should be sure that assert(arg_names.size() == arg_types.size())
                    //
                    this->context->error(call, "Function has no argument named '%s'", entry.first);
                    this->context->shutdown();
                    return;
                }
            }
            call->arguments->named.clear();

            for (size_t i = 0; i < func_type->arg_types.size(); i++) {
                auto existing_value = call->arguments->get_unnamed_value(i);
                if (!existing_value) {
                    auto arg_decl = func->get_arg_declaration(i);

                    if (!arg_decl->value) {
                        this->context->error(call, "There's no value provided for argument '%s'", arg_decl->names[0]);
                        this->context->shutdown();
                        return;
                    }

                    call->arguments->unnamed[i] = arg_decl->value;
                }
            }
        } else {
            // @TODO in case we're calling a function from a variable we should
            // make sure arguments match, no default or named stuff
        }

        for (size_t i = 0; i < call->arguments->unnamed.size(); i++) {
            auto arg_type = func_type->arg_types[i];
            auto value = call->arguments->unnamed[i];
            assert(arg_type->exp_type == AST_EXPRESSION_TYPE);
            auto arg_typed_type = static_cast<Ast_Type*>(arg_type);

            auto success = this->caster->try_implicid_cast(value->inferred_type,
                arg_typed_type, &(call->arguments->unnamed[i]));
            if (!success) {
                this->context->error(value, "Expression cannot be implicitly casted from '%s' to '%s'",
                    value->inferred_type->name, arg_typed_type->name);
                this->context->shutdown();
                return;
            }
        }
    }

    void ast_handle (Ast_Binary** binary_ptr) {
        auto binary = (*binary_ptr);

        if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
            this->ast_handle(&binary->lhs);

            assert(binary->lhs->inferred_type);
            assert(binary->rhs->exp_type == AST_EXPRESSION_IDENT);
            auto ident = static_cast<Ast_Ident*>(binary->rhs);

            auto modified = this->bind_attribute_or_error(binary->lhs->inferred_type, ident, binary_ptr);
            if (modified) {
                this->ast_handle((Ast_Expression**) binary_ptr);
            } else {
                if (!ident->declaration) return;

                this->ast_handle(&binary->rhs);
            }
        } else Ast_Ref_Navigator::ast_handle(binary_ptr);
    }

    void ast_handle (Ast_Unary** unary_ptr) {
        Ast_Ref_Navigator::ast_handle(unary_ptr);

        auto unary = (*unary_ptr);

        assert(unary->exp->inferred_type);
        if (unary->exp->inferred_type == Types::type_type) {
            auto base_type = static_cast<Ast_Type*>(unary->exp);
            auto ptr_type = this->context->type_table->get_or_add_pointer_type(base_type);
            ptr_type->inferred_type = Types::type_type;
            (*unary_ptr) = (Ast_Unary*) ptr_type;
        }
    }

    void ast_handle (Ast_Comma_Separated** comma_separated_ptr) {
        Ast_Ref_Navigator::ast_handle(comma_separated_ptr);

        auto comma_separated = (*comma_separated_ptr);

        bool all_are_types = true;
        bool some_are_types = false;
        For (comma_separated->expressions) {
            if (it->exp_type != AST_EXPRESSION_TYPE) {
                all_are_types = false;
            }
            some_are_types |= (it->exp_type == AST_EXPRESSION_TYPE);
        }

        if (some_are_types && !all_are_types) {
            this->context->error(comma_separated, "Comma separated values cannot have types in them");
            this->context->shutdown();
            return;
        }

        if (all_are_types) {
            auto tuple_type = new Ast_Tuple_Type();
            tuple_type->types = comma_separated->expressions;
            this->context->type_table->unique(&tuple_type);
            (*comma_separated_ptr) = (Ast_Comma_Separated*) tuple_type;
        }
    }

    void ast_handle (Ast_Type** type_ptr) {
        Ast_Ref_Navigator::ast_handle(type_ptr);
        this->type_table->unique(type_ptr);
    }

    void ast_handle (Ast_Array_Type** arr_type_ptr) {
        Ast_Ref_Navigator::ast_handle(arr_type_ptr);

        auto arr_type = (*arr_type_ptr);

        if (arr_type->length->exp_type == AST_EXPRESSION_LITERAL) {
            auto literal = static_cast<Ast_Literal*>(arr_type->length);
            arr_type->length_uint = literal->uint_value;
        } else {
            this->context->error(arr_type->length, "Only literal unsigned integer values allowed in array type size");
            this->context->shutdown();
            return;
        }
    }

    void tuple_try_cast_subtypes (Ast_Type* type, Ast_Expression* value) {
        if (!type || !value) return;

        if (value->exp_type == AST_EXPRESSION_COMMA_SEPARATED && type->typedef_type == AST_TYPEDEF_TUPLE) {
            auto comma_separated = static_cast<Ast_Comma_Separated*>(value);
            auto tuple_type = static_cast<Ast_Tuple_Type*>(type);
            assert(comma_separated->expressions.size == tuple_type->types.size);

            for (size_t i = 0; i < tuple_type->types.size; i++) {
                auto item_value_ptr = &(comma_separated->expressions[i]);
                auto item_value = comma_separated->expressions[i];
                auto item_type_ptr = &(tuple_type->types[i]);
                auto item_type = tuple_type->types[i];

                this->ast_handle(item_value_ptr);
                this->ast_handle(item_type_ptr);

                assert(item_type->exp_type == AST_EXPRESSION_TYPE);
                auto item_typed_type = static_cast<Ast_Type*>(item_type);

                auto success = this->caster->try_implicid_cast(item_value->inferred_type,
                    item_typed_type, item_value_ptr);
                if (!success) {
                    this->context->error(item_value, "Tuple value cannot be implicitly casted from '%s' to '%s'",
                        item_value->inferred_type->name, item_typed_type->name);
                    this->context->shutdown();
                    return;
                }
            }
        }
    }

    bool bind_attribute_or_error (Ast_Type* type, Ast_Ident* ident, Ast_Binary** binary_exp_ptr) {
        switch (type->typedef_type) {
            case AST_TYPEDEF_SLICE:
            case AST_TYPEDEF_STRUCT: {
                auto struct_type = static_cast<Ast_Struct_Type*>(type);
                auto attr_decl = struct_type->find_attribute(ident->name);
                if (attr_decl) {
                    ident->declaration = attr_decl;
                } else {
                    this->context->error(ident, "Struct '%s' has no attribute named '%s'", type->name, ident->name);
                    this->context->shutdown();
                    return false;
                }
                break;
            }
            case AST_TYPEDEF_POINTER: {
                while (type->typedef_type == AST_TYPEDEF_POINTER) {
                    auto tmp = static_cast<Ast_Pointer_Type*>(type);
                    assert(tmp->base->exp_type == AST_EXPRESSION_TYPE);
                    type = tmp->typed_base;

                    auto left_exp_ptr = &(*binary_exp_ptr)->lhs;
                    auto tmp2 = (Ast_Ident*) new Ast_Unary(AST_UNARY_DEREFERENCE, *left_exp_ptr);
                    tmp2->location = (*left_exp_ptr)->location;
                    (*left_exp_ptr) = tmp2;
                }

                this->bind_attribute_or_error(type, ident, binary_exp_ptr);

                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                this->context->error(ident, "Attribute access cannot be performed on function types");
                this->context->shutdown();
                return false;
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                auto array_type = static_cast<Ast_Array_Type*>(type);
                if (strcmp(ident->name, "length") == 0) {
                    auto length_literal = new Ast_Literal(array_type->length_uint);
                    length_literal->location = ident->location;
                    this->inferrer->infer(length_literal);
                    (*binary_exp_ptr) = (Ast_Binary*) length_literal;
                    return true;
                } else if (strcmp(ident->name, "data") == 0) {
                    auto lhs = (*binary_exp_ptr)->lhs;
                    lhs->inferred_type = array_type->typed_base;
                    auto data_ptr = new Ast_Unary(AST_UNARY_REFERENCE, lhs);
                    data_ptr->location = lhs->location;
                    this->inferrer->infer(data_ptr);
                    (*binary_exp_ptr) = (Ast_Binary*) data_ptr;
                    return true;
                } else {
                    this->context->error(ident, "Array types doesn't have a '%s' attribute", ident->name);
                    this->context->shutdown();
                    return false;
                }
                break;
            }
        }
        return false;
    }
};
