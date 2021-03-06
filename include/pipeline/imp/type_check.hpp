#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_ref_navigator.hpp"

#include "pipeline/service/type_inferrer.hpp"
#include "pipeline/service/type_table.hpp"
#include "pipeline/service/type_caster.hpp"

#include <stdint.h>

struct Type_Check : Compiler_Pipe<Ast_Statement*>, Ast_Ref_Navigator {
    Type_Inferrer* inferrer = NULL;
    Type_Table* type_table = NULL;
    Type_Caster* caster = NULL;

    String_Map<Array<Ast_Declaration*>> decl_map;

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
        if (scope->scope_flags & SCOPE_FLAG_TYPES_CHECKED) return;

        decl_map.clear();
        Ast_Utils::get_all_declarations(scope, &decl_map);

        for (auto entry : decl_map) {
            if (entry.second.size > 1) {
                this->context->error(entry.second[0], "Multiple declarations of '%s' found for in the same scope:", entry.first);
                for (size_t i = 1; i < entry.second.size; i++) {
                    this->context->error(entry.second[i], "Re-declaration of '%s' here", entry.first);
                }
                this->context->shutdown();
                return;
            }
        }

        Ast_Ref_Navigator::ast_handle(scope);
        scope->scope_flags |= SCOPE_FLAG_TYPES_CHECKED;
    }

    void ast_handle (Ast_Assign* assign) {
        Ast_Ref_Navigator::ast_handle(assign);

        if (!this->defines_storage(assign->variable)) {
            this->context->error(assign->variable, "Right part of the assignment doesn't declare storage");
            this->context->shutdown();
            return;
        }

        // @TODO check if the variable side of the assignment has storage
        // if not it's a compiler error

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
        auto func = Ast_Utils::get_parent_function(ret->scope);
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

        if (typed_ret_type != this->type_table->type_void) {
            this->resolve_defaults_and_named(ret->result, func->ret_scope);
            this->tuple_try_cast_subtypes(typed_ret_type, (Ast_Expression**) &ret->result);
            Ast_Ref_Navigator::ast_handle(ret);

            auto success = this->caster->try_implicid_cast(ret->result->inferred_type,
                typed_ret_type, (Ast_Expression**) &ret->result);
            if (!success) {
                this->context->error(ret, "Return value cannot be implicitly casted from '%s' to '%s'",
                    ret->result->inferred_type->name, typed_ret_type->name);
                this->context->shutdown();
                return;
            }
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

                this->tuple_try_cast_subtypes(decl_type, &decl->value);
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
                this->ast_handle(&decl->type);
            }
        } else this->ast_handle(&decl->type);

        assert(decl->type);
        assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
    }

    void ast_handle (Ast_Expression** exp_ptr) {
        if (!(*exp_ptr)) return;

        Ast_Ref_Navigator::ast_handle(exp_ptr);
        this->inferrer->infer(*exp_ptr);
        //assert((*exp_ptr)->inferred_type);
    }

    void ast_handle (Ast_Ident** ident_ptr) {
        auto ident = (*ident_ptr);

        assert(ident->declaration);
        auto decl = ident->declaration;

        if (decl->is_constant && decl->value != NULL) {
            switch (decl->value->exp_type) {
                case AST_EXPRESSION_FUNCTION:
                case AST_EXPRESSION_TYPE: {
                    (*ident_ptr) = (Ast_Ident*) decl->value;
                    break;
                }
                default: break;
            }
        }
    }

    void ast_handle (Ast_Function** func_ptr) {
        auto func = (*func_ptr);

        For3 (func->ret_scope->statements, stm ,j) {
            assert(stm->stm_type == AST_STATEMENT_DECLARATION);
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->type) continue;

            if (decl->value) {
                this->ast_handle(&decl->value);
                assert(decl->value->inferred_type);
                decl->type = decl->value->inferred_type;
            } else {
                this->context->error(decl, "Declaration must have a type or a value");
                this->context->shutdown();
                return;
            }
        }

        Ast_Ref_Navigator::ast_handle(func_ptr);

        // @TODO check that all control paths return a value in case the
        // return type is not void
    }

    void ast_handle (Ast_Function_Call** call_ptr) {
        auto call = (*call_ptr);

        Ast_Ref_Navigator::ast_handle(call_ptr);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        if (call->func->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(call->func);
            this->resolve_defaults_and_named(call->arguments, func->arg_scope);
        } else {
            // @TODO in case we're calling a function from a variable we should
            // make sure arguments match, no default or named stuff
        }

        for (size_t i = 0; i < call->arguments->expressions.size; i++) {
            auto arg_type = func_type->arg_types[i];
            assert(arg_type->exp_type == AST_EXPRESSION_TYPE);
            auto arg_typed_type = static_cast<Ast_Type*>(arg_type);

            auto value = call->arguments->expressions[i];
            if (!value) return;

            auto success = this->caster->try_implicid_cast(value->inferred_type,
                arg_typed_type, &(call->arguments->expressions[i]));
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
        if (unary->exp->inferred_type == this->type_table->type_type) {
            auto base_type = static_cast<Ast_Type*>(unary->exp);
            auto ptr_type = this->context->type_table->get_or_add_pointer_type(base_type);
            (*unary_ptr) = (Ast_Unary*) ptr_type;
            Ast_Ref_Navigator::ast_handle((Ast_Expression**) unary_ptr);
        }
    }

    void ast_handle (Ast_Comma_Separated** comma_separated_ptr) {
        Ast_Ref_Navigator::ast_handle(comma_separated_ptr);

        auto comma_separated = (*comma_separated_ptr);
        if (comma_separated->expressions.size == 0) return;

        bool all_are_types = true;
        bool some_are_types = false;
        For (comma_separated->expressions) {
            if (!it) continue;

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
            tuple_type->path = comma_separated->path;
            this->context->type_table->unique(&tuple_type);
            (*comma_separated_ptr) = (Ast_Comma_Separated*) tuple_type;
        }
    }

    void ast_handle (Ast_Type** type_ptr) {
        auto type = (*type_ptr);

        if (type->type_flags & TYPE_FLAG_TYPE_CHECKED) return;

        Ast_Ref_Navigator::ast_handle(type_ptr);
        this->type_table->unique(type_ptr);
        this->compute_type_size(*type_ptr);

        type->type_flags |= TYPE_FLAG_TYPE_CHECKED;
    }

    void ast_handle (Ast_Struct_Type** struct_type_ptr) {
        auto struct_type = (*struct_type_ptr);

        Ast_Ref_Navigator::ast_handle(struct_type_ptr);
        For2 (struct_type->scope.statements, stm) {
            assert(stm->stm_type == AST_STATEMENT_DECLARATION);
            auto decl = static_cast<Ast_Declaration*>(stm);

            assert(decl->names.size > 0);
            assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
            assert(decl->type->inferred_type == this->type_table->type_type);
            assert(decl->typed_type->byte_size > 0);
            if (decl->value) {
                assert(decl->value->inferred_type);
            }
        }
    }

    void ast_handle (Ast_Pointer_Type** ptr_type_ptr) {
        auto ptr_type = (*ptr_type_ptr);

        if (ptr_type->typed_base->typedef_type != AST_TYPEDEF_STRUCT) {
            Ast_Ref_Navigator::ast_handle(ptr_type_ptr);
        }

        ptr_type->byte_size = this->context->target_arch->register_size;
    }

    void ast_handle (Ast_Function_Type** func_type_ptr) {
        Ast_Ref_Navigator::ast_handle(func_type_ptr);
        (*func_type_ptr)->byte_size = this->context->target_arch->register_size;
    }

    void ast_handle (Ast_Array_Type** arr_type_ptr) {
        Ast_Ref_Navigator::ast_handle(arr_type_ptr);

        auto arr_type = (*arr_type_ptr);
        assert(arr_type->base->exp_type == AST_EXPRESSION_TYPE);
        assert(arr_type->base->inferred_type == this->type_table->type_type);

        if (arr_type->length->exp_type != AST_EXPRESSION_LITERAL) {
            this->context->error(arr_type->length, "Only literal unsigned integer values allowed in array type size");
            this->context->shutdown();
            return;
        }
        
        auto literal = static_cast<Ast_Literal*>(arr_type->length);
        arr_type->length_uint = literal->uint_value;

        auto base_type_size = arr_type->typed_base->byte_size;
        assert(base_type_size > 0);
        
        arr_type->byte_size = arr_type->length_uint * base_type_size;
    }

    void compute_type_size (Ast_Type* type) {
        if (type->type_flags & TYPE_FLAG_SIZED) return;

        switch (type->typedef_type) {
            case AST_TYPEDEF_SLICE:
            case AST_TYPEDEF_STRUCT: {
                auto struct_type = static_cast<Ast_Struct_Type*>(type);
                this->compute_struct_type_size(struct_type);
                break;
            }
            case AST_TYPEDEF_FUNCTION:
            case AST_TYPEDEF_POINTER: {
                type->byte_size = this->context->target_arch->register_size;
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                auto arr_type = static_cast<Ast_Array_Type*>(type);

                auto base_type_size = arr_type->typed_base->byte_size;
                assert(base_type_size > 0);
                
                arr_type->byte_size = arr_type->length_uint * base_type_size;
                break;
            }
            case AST_TYPEDEF_TUPLE: {
                auto tuple_type = static_cast<Ast_Tuple_Type*>(type);

                For (tuple_type->types) {
                    assert(it->exp_type == AST_EXPRESSION_TYPE);
                    auto sub_type = tuple_type->typed_types[i];
                    this->compute_type_size(sub_type);
                    tuple_type->byte_size += sub_type->byte_size;
                }

                break;
            }
            default: assert(false);
        }

        type->type_flags |= TYPE_FLAG_SIZED;
    }

    void compute_struct_type_size (Ast_Struct_Type* struct_type) {
        struct_type->byte_size = 0;
        struct_type->byte_padding = 0;
        For2 (struct_type->scope.statements, stm) {
            if (stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm);
                assert(decl->type->exp_type == AST_EXPRESSION_TYPE);
                if (decl->is_constant) continue;

                auto decl_type = static_cast<Ast_Type*>(decl->type);
                assert(decl_type->byte_size > 0);

                decl->attribute_byte_offset = struct_type->byte_size;

                struct_type->byte_size += decl_type->byte_size;
                if ((struct_type->byte_size % 8) > 0) {
                    auto padding = 8 - (struct_type->byte_size % 8);
                    struct_type->byte_padding += padding;
                    struct_type->byte_size += padding;
                }
            } else assert(false);
        }
        assert((struct_type->byte_size % 8) == 0);
    }

    void tuple_try_cast_subtypes (Ast_Expression* type_exp, Ast_Expression** value_ptr) {
        if (!type_exp || !(*value_ptr)) return;

        assert(type_exp->exp_type == AST_EXPRESSION_TYPE);
        auto type = static_cast<Ast_Type*>(type_exp);
        auto value = (*value_ptr);

        if (value->exp_type == AST_EXPRESSION_COMMA_SEPARATED) {
            auto comma_separated = static_cast<Ast_Comma_Separated*>(value);

            if (type->typedef_type == AST_TYPEDEF_TUPLE) {
                auto tuple_type = static_cast<Ast_Tuple_Type*>(type);

                if (comma_separated->expressions.size != tuple_type->types.size) {
                    this->context->error(value, "Mismatch between number of values provided (%zd) and required (%zd)",
                        comma_separated->expressions.size, tuple_type->types.size);
                    this->context->shutdown();
                    return;
                }

                for (size_t i = 0; i < tuple_type->types.size; i++) {
                    auto item_value_ptr = &(comma_separated->expressions[i]);
                    auto item_value = comma_separated->expressions[i];
                    auto item_type_ptr = &(tuple_type->types[i]);
                    auto item_type = tuple_type->types[i];

                    if (!item_value) continue;

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
            } else {
                assert(comma_separated->expressions.size == 1);
                auto item_value_ptr = &(comma_separated->expressions[0]);
                auto item_value = comma_separated->expressions[0];

                this->ast_handle(item_value_ptr);
                this->ast_handle(&type);

                auto success = this->caster->try_implicid_cast(item_value->inferred_type,
                    type, item_value_ptr);
                if (!success) {
                    this->context->error(item_value, "Value cannot be implicitly casted from '%s' to '%s'",
                        item_value->inferred_type->name, type->name);
                    this->context->shutdown();
                    return;
                }
            }
        }
    }

    void resolve_defaults_and_named (Ast_Comma_Separated* args, Ast_Scope* resolver) {
        auto decl_count = resolver->statements.size;

        if (args->expressions.size > decl_count) {
            this->context->error(args, "Too many arguments, should have at most %zd", decl_count);
            this->context->shutdown();
            return;
        }

        for (size_t i = args->expressions.size; i < decl_count; i++) {
            args->expressions.push(NULL);
        }

        for (auto entry : args->named_expressions) {
            auto arg_index = this->get_arg_index(resolver, entry.first);
            if (arg_index != SIZE_MAX) {
                auto existing_value = args->get_unnamed_value(arg_index);
                if (existing_value) {
                    this->context->error(args, "Multiple values provided for argument '%s'", entry.first);
                    this->context->shutdown();
                    return;
                } else {
                    args->expressions[arg_index] = entry.second;
                }
            } else {
                //
                // @Bug @TODO this could produce incorrect error messages, since
                // function types contain the argument names, but are also uniqued!
                // possible solution: the argument names should be stored in the Ast_Function
                // nodes, but we should be sure that assert(arg_names.size() == arg_types.size())
                //
                this->context->error(args, "Function has no argument named '%s'", entry.first);
                this->context->shutdown();
                return;
            }
        }
        args->named_expressions.clear();

        for (size_t i = 0; i < decl_count; i++) {
            auto existing_value = args->get_unnamed_value(i);
            if (!existing_value) {
                auto arg_decl = this->get_arg_declaration(resolver, i);

                if (!arg_decl->value) {
                    if (arg_decl->names.size > 0) {
                        this->context->error(args, "There's no value provided for argument '%s'", arg_decl->names[0]);
                    } else {
                        this->context->error(args, "There's no value provided for unnamed argument at index %zd", i);
                    }
                    this->context->shutdown();
                    return;
                } else if (arg_decl->names[0]) {

                }

                args->expressions[i] = arg_decl->value;
            }
        }
    }

    bool defines_storage (Ast_Expression* exp) {
        // @TODO this needs improvement!

        switch (exp->exp_type) {
            case AST_EXPRESSION_IDENT:  return true;
            case AST_EXPRESSION_COMMA_SEPARATED: {
                auto comma_separated = static_cast<Ast_Comma_Separated*>(exp);
                For (comma_separated->expressions) {
                    if (!this->defines_storage(it)) {
                        return false;
                    }
                }
                return true;
            }
            case AST_EXPRESSION_BINARY: {
                auto binary = static_cast<Ast_Binary*>(exp);
                switch (binary->binary_op) {
                    case AST_BINARY_ATTRIBUTE: return true;
                    default: {
                        return this->defines_storage(binary->lhs)
                            || this->defines_storage(binary->rhs);
                    }
                }
            }
            default:                    return false;
        }
    }

    size_t get_arg_index (Ast_Scope* scope, const char* _name) {
        For3 (scope->statements, stm, i) {
            assert(stm->stm_type == AST_STATEMENT_DECLARATION);
            auto decl = static_cast<Ast_Declaration*>(stm);

            assert(decl->names.size > 0);
            if (strcmp(decl->names[0], _name) == 0) return i;
        }
        return SIZE_MAX;
    }

    Ast_Declaration* get_arg_declaration (Ast_Scope* scope, size_t index) {
        assert(scope->statements.size > index);

        auto arg_stm = scope->statements[index];
        assert(arg_stm->stm_type == AST_STATEMENT_DECLARATION);
        return static_cast<Ast_Declaration*>(arg_stm);
    }

    bool bind_attribute_or_error (Ast_Type* type, Ast_Ident* ident, Ast_Binary** binary_exp_ptr) {
        switch (type->typedef_type) {
            case AST_TYPEDEF_SLICE:
            case AST_TYPEDEF_STRUCT: {
                auto struct_type = static_cast<Ast_Struct_Type*>(type);
                auto attr_decl = Ast_Utils::find_attribute(struct_type, ident->name);
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
                    tmp2->path = (*left_exp_ptr)->path;
                    tmp2->inferred_type = type;
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
                    length_literal->path = ident->path;
                    this->inferrer->infer(length_literal);
                    (*binary_exp_ptr) = (Ast_Binary*) length_literal;
                    return true;
                } else if (strcmp(ident->name, "data") == 0) {
                    auto lhs = (*binary_exp_ptr)->lhs;
                    lhs->inferred_type = array_type->typed_base;
                    auto data_ptr = new Ast_Unary(AST_UNARY_REFERENCE, lhs);
                    data_ptr->path = lhs->path;
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
            default: assert(false);
        }
        return false;
    }
};
