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
		scope->addType(ASTPrimitiveType::_void, Type::getVoidTy(context));
		scope->addType(ASTPrimitiveType::_i1,   Type::getInt1Ty(context));
		scope->addType(ASTPrimitiveType::_i8,   Type::getInt8Ty(context));
		scope->addType(ASTPrimitiveType::_i16,  Type::getInt16Ty(context));
		scope->addType(ASTPrimitiveType::_i32,  Type::getInt32Ty(context));
		scope->addType(ASTPrimitiveType::_i64,  Type::getInt64Ty(context));
		scope->addType(ASTPrimitiveType::_i128, Type::getInt128Ty(context));
	}

	void onFunction (ASTFunction* fn) {
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
			if (fn->type->retType != ASTPrimitiveType::_void) {
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

	Value* codegen (ASTStatement* stm) {
		if 		(auto obj = dynamic_cast<ASTVariable*>(stm)) 	return codegen(obj, true);
		else if (auto obj = dynamic_cast<ASTBlock*>(stm)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<ASTReturn*>(stm)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<ASTExpression*>(stm))  return codegen(obj);
		else return nullptr;
	}

	Value* codegen (ASTBlock* stms) {
		for(auto const& stm: stms->list) {
			this->codegen(stm);
			if (dynamic_cast<ASTReturn*>(stm)) return nullptr;
		}
		return nullptr;
	}

	Value* codegen (ASTVariable* varDef, bool alloca = false) {
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

	Value* codegen (ASTReturn* ret) {
		this->functionHasReturned = false;
		if (ret->exp != nullptr && this->currentReturnVal != nullptr) {
			Value* retValue = this->codegen(ret->exp);
			builder.CreateStore(retValue, this->currentReturnVal);
		}
		return builder.CreateBr(&this->currentFunction->back());
	}

	Value* codegen (ASTExpression* exp) {
		if 		(auto binop = dynamic_cast<ASTBinop*>(exp))   return codegen(binop);
		else if (auto unop  = dynamic_cast<ASTUnop*>(exp))    return codegen(unop);
		else if (auto con   = dynamic_cast<ASTLiteral*>(exp)) return codegen(con);
		else if (auto call  = dynamic_cast<ASTCall*>(exp)) 	  return codegen(call);
		else if (auto mem  = dynamic_cast<ASTMemory*>(exp))   return codegen(mem, true);
		else return nullptr;
	}

	Value* codegen (ASTBinop* binop) {
		switch (binop->op) {
			case ASTBinop::OP::ADD:
				return LLVMCodegenPrimitive::add(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case ASTBinop::OP::SUB:
				return LLVMCodegenPrimitive::sub(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case ASTBinop::OP::MUL:
				return LLVMCodegenPrimitive::mul(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case ASTBinop::OP::DIV:
				return LLVMCodegenPrimitive::div(&builder,
					this->codegen(binop->lhs), this->codegen(binop->rhs));
			case ASTBinop::OP::ASSIGN: {
				if (auto mem = dynamic_cast<ASTMemory*>(binop->lhs)) {
					return LLVMCodegenPrimitive::assign(&builder,
						this->codegen(mem), this->codegen(binop->rhs));
				} else cout << "Assigment LHS is not memory!\n";
			}
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTUnop* unop) {
		Value* val = this->codegen(unop->exp);
		switch (unop->op) {
			case ASTUnop::OP::NEG:
				return LLVMCodegenPrimitive::neg(&builder, val);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTLiteral* con) {
		switch (con->type) {
			case ASTLiteral::TYPE::INT:
				return ConstantInt::get(context, APInt(32, con->intValue));
			case ASTLiteral::TYPE::STRING:
				return builder.CreateGlobalStringPtr(con->stringValue);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTCall* call) {
		Function* function = nullptr;
		if (auto fn = dynamic_cast<ASTFunction*>(call->fn)) {
			function = this->scope->getFunction(fn);
		}
		if (function == nullptr) cout << "Function not found!";

		vector<Value*> params;
		for(auto const& param: call->params)
			params.push_back(this->codegen(param));
		return builder.CreateCall(function, params, "fnCall");
	}

	Value* codegen(ASTMemory* mem, bool autoDeref = false) {
		Value* val = nullptr;
		if (auto ref  = dynamic_cast<ASTRef*>(mem)) return codegen(ref);
		else if (auto var = dynamic_cast<ASTVariable*>(mem)) {
			val = codegen(var);
		} else if (auto attr = dynamic_cast<ASTAttr*>(mem)) {
			val = codegen(attr);
		} else if (auto der = dynamic_cast<ASTDeref*>(mem)) {
			val = codegen(der);
		} else return nullptr;
		if (autoDeref) val =  builder.CreateLoad(val);
		return val;
	}

	Value* codegen (ASTRef* ref) {
		return this->codegen(ref->memory);
	}

	Value* codegen (ASTAttr* attr) {
		Value* val = this->codegen(attr->exp);
		Type* ty = val->getType();
		if (ty->isPointerTy()) {
			PointerType* pTy = static_cast<PointerType*>(ty);
			Type* actualType = pTy->getElementType();
			if (actualType->isStructTy()) {
				//return builder.CreateStructGEP(actualType, val, 0);
				Value* attrPointer = builder.CreateStructGEP(actualType, val, 0);
				return builder.CreateLoad(attrPointer, "attr." + attr->name);
			}
		} else cout << "[ASTAttr::codegen] value type is not a struct!";
		return nullptr;
	}

	Value* codegen (ASTDeref* deref) {
		auto val = this->codegen(deref->memory);
		return builder.CreateLoad(val, "");
	}

	void onType (ASTType* ty) {
		this->codegen(ty);
	}

	Type* codegen (ASTType* ty) {
		auto cachedTy = this->scope->getType(ty);
		if (cachedTy) return cachedTy;

		if (auto obj = dynamic_cast<ASTStructType*>(ty)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<ASTPointerType*>(ty))  return codegen(obj);
		else cout << "ERROR\n\n";
		return nullptr;
	}

	Type* codegen (ASTPointerType* ty) {
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

	Type* codegen (ASTStructType* ty) {
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
		module->print(outs(), nullptr);
		auto start = clock();
		LLVMObjWritter::writeObj(module);
		Timer::print("  Write OBJ ", start);
	}
};
