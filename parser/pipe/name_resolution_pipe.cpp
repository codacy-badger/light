#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include "../pipes.cpp"
#include "parser/ast/ast.hpp"

struct AddrDeps {
	std::map<std::string, std::set<void**>> addrs;

	bool addIfUnresolved (ASTType** ty) {
		return this->addIfUnresolved(reinterpret_cast<ASTExpression**>(ty));
	}

	bool addIfUnresolved (ASTMemory** mem) {
		return this->addIfUnresolved(reinterpret_cast<ASTExpression**>(mem));
	}

	bool addIfUnresolved (ASTExpression** exp) {
		auto un = dynamic_cast<ASTUnresolved*>(*exp);
		if (un) {
			this->add(un->name, exp);
			return true;
		} else return false;
	}

	void add (std::string name, ASTExpression** exp) {
		auto vAddr = reinterpret_cast<void**>(exp);
		addrs[name].insert(vAddr);
	}

	void print() {
		for (auto const &entry : this->addrs) {
			cout << " + " << entry.first << "\n";
			for (auto const &addr : entry.second) {
				cout << "    -> " << addr << "\n";
			}
		}
	}
};

struct NameResolutionPipe : Pipe {
	void onFunction (ASTFunction* fn) {
		AddrDeps deps;
		check(fn, &deps);
		if (deps.addrs.size() > 0) {
			cout << "Function -> " << fn->name << "\n";
			deps.print();
		}
		this->toNext(fn);
	}

	void onType (ASTType* ty) {
		AddrDeps deps;
		check(ty, &deps);
		if (deps.addrs.size() > 0) {
			cout << "Type\n";
			deps.print();
		}
		this->toNext(ty);
	}

	void check (ASTFunction* fn, AddrDeps* deps) {
		check(fn->type, deps);
		check(fn->stm, deps);
	}

	void check (ASTStatement* stm, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<ASTVariable*>(stm))	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTReturn*>(stm))		check(obj, deps);
		else if (auto obj = dynamic_cast<ASTBlock*>(stm))		check(obj, deps);
		else if (auto obj = dynamic_cast<ASTExpression*>(stm))	check(obj, deps);
	}

	void check (ASTBlock* block, AddrDeps* deps) {
		for (auto const& stm : block->list) check(stm, deps);
	}

	void check (ASTVariable* var, AddrDeps* deps) {
		if (var->type) {
			if (!deps->addIfUnresolved(&var->type))
				check(var->type, deps);
		}
		if (var->expression) check(var->expression, deps);
	}

	void check (ASTReturn* ret, AddrDeps* deps) {
		if (ret->exp) check(ret->exp, deps);
	}

	void check (ASTExpression* exp, AddrDeps* deps) {
		if		(auto obj = dynamic_cast<ASTBinop*>(exp)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTUnop*>(exp))	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTValue*>(exp))	check(obj, deps);
	}

	void check (ASTBinop* binop, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&binop->lhs)) check(binop->lhs, deps);
		if (!deps->addIfUnresolved(&binop->rhs)) check(binop->rhs, deps);
	}

	void check (ASTUnop* unop, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&unop->exp)) check(unop->exp, deps);
	}

	void check (ASTValue* val, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<ASTCall*>(val)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTLiteral*>(val))	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTMemory*>(val))	check(obj, deps);
	}

	void check (ASTCall* call, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&call->fn)) check(call->fn, deps);
		for (auto const& param : call->params) check(param, deps);
	}

	void check (ASTLiteral* lit, AddrDeps* deps) {
		return /* noop */;
	}

	void check (ASTMemory* mem, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<ASTVariable*>(mem))	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTDeref*>(mem))		check(obj, deps);
		else if (auto obj = dynamic_cast<ASTRef*>(mem))			check(obj, deps);
		else if (auto obj = dynamic_cast<ASTAttr*>(mem))		check(obj, deps);
	}

	void check (ASTDeref* deref, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&deref->memory)) check(deref->memory, deps);
	}

	void check (ASTRef* ref, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ref->memory)) check(ref->memory, deps);
	}

	void check (ASTAttr* attr, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&attr->exp)) check(attr->exp, deps);
	}

	void check (ASTType* ty, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<ASTStructType*>(ty)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<ASTPointerType*>(ty))  check(obj, deps);
		else if (auto obj = dynamic_cast<ASTFnType*>(ty))  		check(obj, deps);
	}

	void check (ASTStructType* ty, AddrDeps* deps) {
		for (auto const& attr : ty->attrs) check(attr, deps);
	}

	void check (ASTPointerType* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->base)) this->check(ty->base, deps);
	}

	void check (ASTFnType* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->retType)) this->check(ty->retType, deps);
	}
};
