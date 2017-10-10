#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include "../pipes.cpp"
#include "parser/ast/ast.hpp"

struct ExpDesp {
	ASTExpression* exp;
	std::set<std::string> names;
};

struct AddrDeps {
	std::map<std::string, std::set<void**>> addrs;

	template <typename T>
	bool addIfUnresolved (T** exp) {
		return this->addIfUnresolved(reinterpret_cast<ASTExpression**>(exp));
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
	std::map<std::string, std::set<void**>> ptrDeps;
	std::map<std::string, ExpDesp> astDeps;

	void onFunction (ASTFunction* fn) {
		AddrDeps deps;
		check(fn, &deps);
	}

	void onType (ASTType* ty) {
		AddrDeps deps;
		check(ty, &deps);
	}

	void onFinish () {
		if (astDeps.size() > 0) {
			for (auto const &entry : astDeps)
				cout << "ERROR: unresolved symbol: " << entry.first << "\n";
		} else if (this->next) this->next->onFinish();
	}

	void check (ASTFunction* fn, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&fn->type)) check(fn->type, deps);
		if (!deps->addIfUnresolved(&fn->stm))  check(fn->stm, deps);
		if (deps->addrs.size() > 0) {
			cout << "Function -> " << fn->name << "\n";
			deps->print();
			for (auto const &entry : deps->addrs) {
				astDeps[entry.first].exp = fn;
				for (auto const &entry2 : deps->addrs)
					astDeps[entry.first].names.insert(entry2.first);
			}
		} else {
			this->toNext(fn);
			//TODO: trigger name resolution in case is possible
		}
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
		else if (auto obj = dynamic_cast<ASTMemory*>(val))	check(obj, deps);
	}

	void check (ASTCall* call, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&call->fn)) check(call->fn, deps);
		for (auto const& param : call->params) check(param, deps);
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
		if (deps->addrs.size() > 0) {
			cout << "Type -> " << ty->name << "\n";
			deps->print();
			astDeps[ty->name].exp = ty;
			for (auto const &entry : deps->addrs)
				astDeps[ty->name].names.insert(entry.first);
		} else {
			this->toNext(ty);
			//TODO: trigger name resolution in case is possible
		}
	}

	void check (ASTPointerType* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->base)) this->check(ty->base, deps);
	}

	void check (ASTFnType* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->retType)) this->check(ty->retType, deps);
	}
};
