#include "pipeline/pipeline.hpp"

#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/import_modules.hpp"

#include "bytecode/pipe/register_allocator.hpp"
#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

#include <algorithm>

#define COMPILER_TOTAL_FORMAT "\n  Total Time                 %8.6fs\n"

#define DECL_TYPE(type) this->global_scope->list.push_back(ast_make_declaration(type->name, type));

bool sort_pipes (const Pipe* lhs, const Pipe* rhs) {
    return lhs->priority < rhs->priority;
}

Pipeline::Pipeline() {
    os_get_current_directory(this->parser->current_path);

    this->pipes.push_back(new Foreign_Function());
    this->pipes.push_back(new Symbol_Resolution());
    this->pipes.push_back(new Type_Checking());
    this->pipes.push_back(new Import_Modules());
    this->pipes.push_back(new Constant_Folding());

    this->pipes.push_back(new Register_Allocator());
    this->pipes.push_back(new Bytecode_Generator());
    this->pipes.push_back(new Bytecode_Runner());

    // @Incomplete: add output pipes (DLL, EXE, etc.)

    sort(this->pipes.begin(), this->pipes.begin(), sort_pipes);

    this->global_scope = this->parser->factory->new_node<Ast_Block>();
    DECL_TYPE(Types::type_def_type);
    DECL_TYPE(Types::type_def_void);
    DECL_TYPE(Types::type_def_bool);
    DECL_TYPE(Types::type_def_s8);
    DECL_TYPE(Types::type_def_s16);
    DECL_TYPE(Types::type_def_s32);
    DECL_TYPE(Types::type_def_s64);
    DECL_TYPE(Types::type_def_u8);
    DECL_TYPE(Types::type_def_u16);
    DECL_TYPE(Types::type_def_u32);
    DECL_TYPE(Types::type_def_u64);
    DECL_TYPE(Types::type_def_f32);
    DECL_TYPE(Types::type_def_f64);
}

void Pipeline::run(const char* filepath) {
    printf("\n%s\n", filepath);

    auto start = os_get_user_time();

    this->handle_file(filepath);

    print_compiler_metrics(os_time_user_stop(start));
}

void Pipeline::handle_file(const char* filepath) {
    auto last_lexer = this->parser->setup(filepath, this->global_scope);

	auto start = os_get_user_time();
    auto stm = this->parser->statement();
	this->parser->total_time += os_time_user_stop(start);

    while (stm != NULL) {
        this->parser->add(stm);

        this->handle_stm(stm);

        start = os_get_user_time();
        stm = this->parser->statement();
    	this->parser->total_time += os_time_user_stop(start);
    }

    for (auto pipe : this->pipes) {
        pipe->on_finish();
    }

    this->parser->teardown(last_lexer);
}

void Pipeline::handle_stm(Ast_Statement* stm, int from_index) {
    for (int i = from_index; i < this->pipes.size(); i++) {
        auto pipe = this->pipes[i];

        pipe->on_statement(&stm);

        while (pipe->pending_stms.size()) {
            auto _stm = pipe->pending_stms.front();
            pipe->pending_stms.pop_front();

            this->handle_stm(_stm, i + 1);
        }

        while (this->pending_imports.size()) {
            auto import = this->pending_imports.front();
            this->pending_imports.pop_front();

            // @Incomplete don't assume expression is a string
            auto literal = static_cast<Ast_Literal*>(import->target);
            this->handle_file(literal->string_value);
        }

        if (pipe->stop_processing) {
            pipe->stop_processing = false;
            break;
        }
    }
}

void Pipeline::print_compiler_metrics (double total_time) {
	printf("\n");

    double percent = (this->parser->total_time * 100.0) / total_time;
    printf("  - %-25s%8.6fs (%5.2f%%)\n", this->parser->pipe_name,
        this->parser->total_time, percent);
    this->parser->print_pipe_metrics();

    for (auto pipe : this->pipes) {
        if (pipe->pipe_name) {
			percent = (pipe->total_time * 100.0) / total_time;
			printf("  - %-25s%8.6fs (%5.2f%%)\n", pipe->pipe_name, pipe->total_time, percent);
			pipe->print_pipe_metrics();
		}
    }
	printf(COMPILER_TOTAL_FORMAT, total_time);
}
