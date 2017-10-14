#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/SourceMgr.h"

#include <string>

#include "timer.cpp"
#include "parser/pipes.cpp"
#include "codegen_primitive.cpp"
#include "llvm_obj_writter.cpp"
#include "llvm_scope.cpp"

struct LLVMPipe : Pipe {
	LLVMContext context;
	IRBuilder<> builder;
	Module* module;
	LLVMScope* scope;

	Function* currentFunction;
	AllocaInst* currentReturnVal;
	bool functionHasReturned;

	LLVMPipe (std::string output) : builder(context) {
		module = new Module(output, context);
		scope = new LLVMScope(&builder);
		scope->addType(Ast_Primitive_Type::_void, Type::getVoidTy(context));
		scope->addType(Ast_Primitive_Type::_i1,   Type::getInt1Ty(context));
		scope->addType(Ast_Primitive_Type::_i8,   Type::getInt8Ty(context));
		scope->addType(Ast_Primitive_Type::_i16,  Type::getInt16Ty(context));
		scope->addType(Ast_Primitive_Type::_i32,  Type::getInt32Ty(context));
		scope->addType(Ast_Primitive_Type::_i64,  Type::getInt64Ty(context));
		scope->addType(Ast_Primitive_Type::_i128, Type::getInt128Ty(context));
	}

	void onFunction (Ast_Function* fn) {
		vector<Type*> argTypes;
		for(auto const& param: fn->type->params)
			argTypes.push_back(this->codegen(param->type));
		Type* retType = this->codegen(fn->type->retType);
		auto fnType = FunctionType::get(retType, argTypes, false);
		auto constValue = module->getOrInsertFunction(fn->name, fnType);
		auto function = static_cast<Function*>(constValue);

		int index = 0;
		for (auto& arg: function->args())
			arg.setName(fn->type->params[index++]->name);
		this->scope->addFunction(fn, function);

		if (fn->stm != nullptr) {
			BasicBlock* prevBlock = builder.GetInsertBlock();
			this->functionHasReturned = false;
			this->currentFunction = function;

			BasicBlock* entryBlock = BasicBlock::Create(context, "entry", function);
			BasicBlock* retBlock = BasicBlock::Create(context, "return", function);
			if (fn->type->retType != Ast_Primitive_Type::_void) {
				builder.SetInsertPoint(entryBlock);
				auto type = this->codegen(fn->type->retType);
				this->currentReturnVal = builder.CreateAlloca(type, nullptr, "retVal");
				builder.SetInsertPoint(retBlock);
				auto retVal = builder.CreateLoad(this->currentReturnVal);
				builder.CreateRet(retVal);
			} else {
				this->currentReturnVal = nullptr;
				builder.SetInsertPoint(retBlock);
				builder.CreateRetVoid();
			}

			builder.SetInsertPoint(entryBlock);
			this->scope = scope->push();
			this->scope->addParameters(fn);
			this->codegen(fn->stm);
			this->scope = scope->pop();

			if (!builder.GetInsertBlock()->back().isTerminator()) {
				builder.CreateBr(retBlock);
			}
			builder.SetInsertPoint(prevBlock);
		}

		verifyFunction(*function);
	}

	Value* codegen (Ast_Statement* stm) {
		if 		(auto obj = dynamic_cast<Ast_Variable*>(stm)) 	return codegen(obj, true);
		else if (auto obj = dynamic_cast<Ast_Block*>(stm)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<Ast_Return*>(stm)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<Ast_Expression*>(stm))  return codegen(obj);
		else return nullptr;
	}

	Value* codegen (Ast_Block* stms) {
		for(auto const& stm: stms->list) {
			this->codegen(stm);
			if (dynamic_cast<Ast_Return*>(stm)) return nullptr;
		}
		return nullptr;
	}

	Value* codegen (Ast_Variable* varDef, bool alloca = false) {
		if (alloca) {
			auto type = this->codegen(varDef->type);
			auto alloca = builder.CreateAlloca(type, nullptr, varDef->name);
			if (varDef->expression != NULL) {
				Value* val = this->codegen(varDef->expression);
				builder.CreateStore(val, alloca);
			}
			this->scope->addVariable(varDef, alloca);
			return alloca;
		} else return this->scope->getVariable(varDef);
	}

	Value* codegen (Ast_Return* ret) {
		this->functionHasReturned = false;
		if (ret->exp != nullptr && this->currentReturnVal != nullptr) {
			Value* retValue = this->codegen(ret->exp);
			builder.CreateStore(retValue, this->currentReturnVal);
		}
		return builder.CreateBr(&this->currentFunction->back());
	}

	Value* codegen (Ast_Expression* exp) {
		if 		(auto binop = dynamic_cast<AST_Binary*>(exp))   return codegen(binop);
		else if (auto unop  = dynamic_cast<AST_Unary*>(exp))    return codegen(unop);
		else if (auto con   = dynamic_cast<Ast_Literal*>(exp)) return codegen(con);
		else if (auto call  = dynamic_cast<Ast_Function_Call*>(exp)) 	  return codegen(call);
		else if (auto mem  = dynamic_cast<AST_Memory*>(exp))   return codegen(mem, true);
		else return nullptr;
	}

	Value* codegen (AST_Binary* binop) {
		switch (binop->op) {
			case AST_Binary::OP::ADD:
				return LLVMCodegenPrimitive::add(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case AST_Binary::OP::SUB:
				return LLVMCodegenPrimitive::sub(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case AST_Binary::OP::MUL:
				return LLVMCodegenPrimitive::mul(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case AST_Binary::OP::DIV:
				return LLVMCodegenPrimitive::div(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case AST_Binary::OP::ASSIGN: {
				if (auto mem = dynamic_cast<AST_Memory*>(binop->lhs)) {
					return LLVMCodegenPrimitive::assign(&builder,
						this->codegen(mem), this->codegen(binop->rhs));
				} else cout << "Assigment LHS is not memory!\n";
			}
			default: break;
		}
		return nullptr;
	}

	Value* codegen (AST_Unary* unop) {
		Value* val = this->codegen(unop->exp);
		switch (unop->op) {
			case AST_Unary::OP::NEG:
				return LLVMCodegenPrimitive::neg(&builder, val);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (Ast_Literal* con) {
		switch (con->type) {
			case Ast_Literal::TYPE::INT:
				return ConstantInt::get(context, APInt(32, con->intValue));
			case Ast_Literal::TYPE::STRING:
				return builder.CreateGlobalStringPtr(con->stringValue);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (Ast_Function_Call* call) {
		Function* function = nullptr;
		if (auto fn = dynamic_cast<Ast_Function*>(call->fn)) {
			function = this->scope->getFunction(fn);
		}
		if (function == nullptr) cout << "Function not found!";

		vector<Value*> params;
		for(auto const& param: call->params)
			params.push_back(this->codegen(param));
		return builder.CreateCall(function, params, "fnCall");
	}

	Value* codegen(AST_Memory* mem, bool autoDeref = false) {
		Value* val = nullptr;
		if (auto ref  = dynamic_cast<AST_Ref*>(mem)) return codegen(ref);
		else if (auto var = dynamic_cast<Ast_Variable*>(mem)) {
			val = codegen(var);
		} else if (auto attr = dynamic_cast<Ast_Attribute*>(mem)) {
			val = codegen(attr);
		} else if (auto der = dynamic_cast<Ast_Deref*>(mem)) {
			val = codegen(der);
		} else return nullptr;
		if (autoDeref) val =  builder.CreateLoad(val);
		return val;
	}

	Value* codegen (AST_Ref* ref) {
		return this->codegen(ref->memory);
	}

	Value* codegen (Ast_Attribute* attr) {
		auto mem = dynamic_cast<AST_Memory*>(attr->exp);
		Value* val = this->codegen(mem);
		Type* ty = val->getType();
		if (ty->isPointerTy()) {
			PointerType* ptrTy = static_cast<PointerType*>(ty);
			if (ptrTy->getElementType()->isStructTy()) {
				auto structTy = ptrTy->getElementType();
				return builder.CreateStructGEP(structTy, val, 0);
			} else cout << "[Ast_Attribute::codegen] value type is not a struct!";
		}
		return nullptr;
	}

	Value* codegen (Ast_Deref* deref) {
		auto val = this->codegen(deref->memory);
		return builder.CreateLoad(val, "");
	}

	void onType (Ast_Type_Definition* ty) {
		this->codegen(ty);
	}

	Type* codegen (Ast_Type_Definition* ty) {
		auto cachedTy = this->scope->getType(ty);
		if (cachedTy) return cachedTy;

		if (auto obj = dynamic_cast<Ast_Struct_Type*>(ty)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<Ast_Pointer_Type*>(ty))  return codegen(obj);
		else cout << "ERROR\n\n";
		return nullptr;
	}

	Type* codegen (Ast_Pointer_Type* ty) {
		PointerType* ptrTy = nullptr;
		Type* baseTy = this->codegen(ty->base);
		if (PointerType::isValidElementType(baseTy)) {
			return PointerType::get(baseTy, 0);
		} else {
			outs() << "Invalid type to use in pointer: ";
			baseTy->print(outs());
			exit(1);
		}
		return ptrTy;
	}

	Type* codegen (Ast_Struct_Type* ty) {
		StructType* structTy;
		if (ty->attrs.size() > 0) {
			vector<Type*> attrTypes;
			for (auto &attr : ty->attrs)
				attrTypes.push_back(this->codegen(attr->type));
			structTy = StructType::create(attrTypes, ty->name);
		} else {
			structTy = StructType::create(builder.getContext(), ty->name);
		}
		this->scope->addType(ty, structTy);
		return structTy;
	}

	void onFinish () {
		//module->print(outs(), nullptr);
		auto start = clock();
		LLVMObjWritter::writeObj(module);
		Timer::print("  Write ", start);
	}
};
