#include "modules.hpp"

#include "compiler.hpp"
#include "platform.hpp"

#include "phase/pipeline/pipes/constant_propagation.hpp"
#include "phase/pipeline/pipes/constant_folding.hpp"
#include "phase/pipeline/pipes/cast_strings.hpp"
#include "phase/pipeline/pipes/cast_arrays.hpp"
#include "phase/pipeline/pipes/cast_anys.hpp"
#include "phase/pipeline/pipes/type_checking.hpp"
#include "phase/pipeline/pipes/foreign_function.hpp"
#include "phase/pipeline/pipes/call_arguments.hpp"

#include "phase/pipeline/pipes/register_allocator.hpp"
#include "phase/pipeline/pipes/bytecode_generator.hpp"
#include "phase/pipeline/pipes/run_directive.hpp"

Modules::Modules (Compiler* compiler) {
    this->pipeline
        ->pipe(new Foreign_Function())
        ->pipe(new Constant_Propagation())
        ->pipe(new Cast_Strings())
        ->pipe(new Type_Checking())
        ->pipe(new Cast_Arrays())
        ->pipe(new Cast_Anys())
        ->pipe(new Constant_Folding())
        ->pipe(new Call_Arguments())

        ->pipe(new Register_Allocator(INTERP_REGISTER_COUNT))
        ->pipe(new Bytecode_Generator())
        ->pipe(new Run_Directive(compiler->interp));
}

bool Modules::is_module_cached (const char* absolute_path) {
    auto it = this->cache.find(absolute_path);
    return it != this->cache.end();
}

void Modules::print_metrics (double userInterval) {
	printf("\n");
    this->pipeline->print_metrics(userInterval);
}
