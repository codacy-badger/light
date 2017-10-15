#pragma once

#include "back/llvm/llvm_scope.hpp"

LLVMScope::LLVMScope (IRBuilder<>* builder, LLVMScope* parent) {
	this->builder = builder;
	this->parent = parent;
}

void LLVMScope::addVariable (Ast_Declaration* var) {
	auto type = this->getType(var->type);
	auto alloca = builder->CreateAlloca(type, nullptr, var->name);
	this->addVariable(var, alloca);
}

void LLVMScope::addVariable (Ast_Declaration* var, AllocaInst* alloca) {
	auto it = variables.find(var);
	if (it == variables.end()) variables[var] = alloca;
	else cout << "Variable already exists " << var->name << "\n";
}

void LLVMScope::addParameters (Ast_Function* fn) {
	int i = 0;
	auto function = this->getFunction(fn);
	for (auto &param : fn->type->parameters) {
		auto type = this->getType(param->type);
		auto alloca = builder->CreateAlloca(type, 0, param->name + ".arg");

		Argument* llvmArg = std::next(function->arg_begin(), i++);
		builder->CreateStore(llvmArg, alloca);

		this->addVariable(param, alloca);
	}
}

AllocaInst* LLVMScope::getVariable (Ast_Declaration* var) {
	auto it = variables.find(var);
	if (it == variables.end()) return nullptr;
	else return variables[var];
}

AllocaInst* LLVMScope::getVariable (string name) {
	for(auto it = variables.begin(); it != variables.end(); ++it) {
		if (it->first->name == name) return it->second;
	}
	cout << "Variable " << name << " not found\n";
	return nullptr;
}

void LLVMScope::addType (Ast_Type_Definition* ty, Type* type) {
	auto it = types.find(ty);
	if (it == types.end()) types[ty] = type;
	//TODO: print the name of the type (virtual function?)
	else cout << "Type already exists \n";
}

void LLVMScope::addType (Ast_Type_Definition* alias, Ast_Type_Definition* original) {
	auto it = types.find(original);
	if (it != types.end()) types[alias] = types[original];
	else cout << "Type  not found\n";
}

Type* LLVMScope::getType (Ast_Type_Definition* ty) {
	if (ty == nullptr) return Type::getVoidTy(builder->getContext());
	else if (ty->type_def_type == AST_TYPE_DEF_POINTER) {
		auto ptrTy = static_cast<Ast_Pointer_Type*>(ty);
		auto baseTy = this->getType(ptrTy->base);
		return PointerType::get(baseTy, 0);
	} else {
		auto it = types.find(ty);
		if (it != types.end())
			return types[ty];
		else if (this->parent != nullptr) {
			return this->parent->getType(ty);
		} else return nullptr;
	}
}

void LLVMScope::addFunction (Ast_Function* fn, Function* function) {
	auto it = functions.find(fn);
	if (it == functions.end()) functions[fn] = function;
	else cout << "Function already exists " << fn->name << "\n";
}

Function* LLVMScope::getFunction (Ast_Function* fn) {
	auto it = functions.find(fn);
	if (it != functions.end())
		return functions[fn];
	else if (this->parent != nullptr) {
		return this->parent->getFunction(fn);
	} else return nullptr;
}

LLVMScope* LLVMScope::push () {
	return new LLVMScope(builder, this);
}

LLVMScope* LLVMScope::pop () {
	if (this->parent != nullptr) return this->parent;
	else {
		cout << "ERROR: pop of global scope!";
		return this;
	}
}

map<Ast_Type_Definition*, Type*> LLVMScope::types;
map<Ast_Function*, Function*> LLVMScope::functions;
