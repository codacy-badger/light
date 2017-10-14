#pragma once

#include "back/llvm/codegen_primitive.hpp"

Value* LLVMCodegenPrimitive::assign (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateStore(rhs, lhs);
}

Value* LLVMCodegenPrimitive::add (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateAdd(lhs, rhs, "add");
}

Value* LLVMCodegenPrimitive::sub (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateSub(lhs, rhs, "sub");
}

Value* LLVMCodegenPrimitive::mul (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateMul(lhs, rhs, "mul");
}

Value* LLVMCodegenPrimitive::div (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateSDiv(lhs, rhs, "div");
}

Value* LLVMCodegenPrimitive::neg (IRBuilder<>* builder, Value* val) {
	return builder->CreateNeg(val, "neg");
}
