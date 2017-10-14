#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include "parser/pipes.hpp"

struct ExpDesp {
	Ast_Expression* exp;
	std::set<std::string> names;
};

struct AddrDeps {
	std::map<std::string, std::set<void**>> addrs;

	template <typename T>
	bool addIfUnresolved (T** exp) {
		return this->addIfUnresolved(reinterpret_cast<Ast_Expression**>(exp));
	}

	bool addIfUnresolved (Ast_Expression** exp) {
		auto un = dynamic_cast<Ast_Unresolved*>(*exp);
		if (un) {
			this->add(un->name, exp);
			return true;
		} else return false;
	}

	void add (std::string name, Ast_Expression** exp) {
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
	std::map<std::string, std::set<ExpDesp*>> astDeps;

	void onFunction (Ast_Function* fn) {
		AddrDeps deps;
		check(fn, &deps);
		if (deps.addrs.size() > 0) {
			//cout << "Function -> " << fn->name << "\n";
			//deps->print();
			this->addDependencies(&deps, fn);
		} else {
			this->toNext(fn);
			this->resolve(fn->name, fn);
		}
	}

	void onType (Ast_Type_Definition* ty) {
		AddrDeps deps;
		check(ty, &deps);
		if (deps.addrs.size() > 0) {
			//cout << "Type -> " << ty->name << "\n";
			//deps->print();
			this->addDependencies(&deps, ty);
		} else {
			this->toNext(ty);
			if (auto namedTy = dynamic_cast<Ast_Struct_Type*>(ty)) {
				this->resolve(namedTy->name, namedTy);
			}
		}
	}

	void onFinish () {
		if (astDeps.size() > 0) {
			for (auto const &entry : astDeps) {
				cout << "ERROR: unresolved symbol: " << entry.first << "\n";
				//for (auto const &res : entry.second) res->exp->print();
			}
		} else this->tryFinish();
	}

	void addDependencies (AddrDeps* deps, Ast_Expression* exp) {
		auto expDesp = new ExpDesp();
		expDesp->exp = exp;
		for (auto const &entry : deps->addrs) {
			expDesp->names.insert(entry.first);
			astDeps[entry.first].insert(expDesp);
			ptrDeps[entry.first].insert(entry.second.begin(), entry.second.end());
		}
	}

	void resolve (std::string name, Ast_Expression* exp) {
		auto it = ptrDeps.find(name);
		if (it != ptrDeps.end()) {
			//cout << "Resolving: " << name << "\n";
			for (auto const &entry : ptrDeps[name]) *entry = exp;
			ptrDeps.erase(name);
			for (auto entry : astDeps[name]) {
				entry->names.erase(name);
				if (entry->names.size() == 0) {
					this->toNext(entry->exp);
					if (auto obj = dynamic_cast<Ast_Struct_Type*>(entry->exp)) {
						this->resolve(obj->name, obj);
					} else if (auto obj = dynamic_cast<Ast_Function*>(entry->exp)) {
						this->resolve(obj->name, obj);
					}
				}
			}
			astDeps.erase(name);
		}
	}

	void check (Ast_Function* fn, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&fn->type)) check(fn->type, deps);
		if (!deps->addIfUnresolved(&fn->stm))  check(fn->stm, deps);
	}

	void check (Ast_Statement* stm, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<Ast_Variable*>(stm))	check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Return*>(stm))		check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Block*>(stm))		check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Expression*>(stm))	check(obj, deps);
	}

	void check (Ast_Block* block, AddrDeps* deps) {
		for (auto const& stm : block->list) check(stm, deps);
	}

	void check (Ast_Variable* var, AddrDeps* deps) {
		if (var->type) {
			if (!deps->addIfUnresolved(&var->type))
				check(var->type, deps);
		}
		if (var->expression) check(var->expression, deps);
	}

	void check (Ast_Return* ret, AddrDeps* deps) {
		if (ret->exp) check(ret->exp, deps);
	}

	void check (Ast_Expression* exp, AddrDeps* deps) {
		if		(auto obj = dynamic_cast<AST_Binary*>(exp)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<AST_Unary*>(exp))	check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Value*>(exp))	check(obj, deps);
	}

	void check (AST_Binary* binop, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&binop->lhs)) check(binop->lhs, deps);
		if (!deps->addIfUnresolved(&binop->rhs)) check(binop->rhs, deps);
	}

	void check (AST_Unary* unop, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&unop->exp)) check(unop->exp, deps);
	}

	void check (Ast_Value* val, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<Ast_Function_Call*>(val)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<AST_Memory*>(val))	check(obj, deps);
	}

	void check (Ast_Function_Call* call, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&call->fn)) check(call->fn, deps);
		for (auto const& param : call->params) check(param, deps);
	}

	void check (AST_Memory* mem, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<Ast_Variable*>(mem))	check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Deref*>(mem))		check(obj, deps);
		else if (auto obj = dynamic_cast<AST_Ref*>(mem))			check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Attribute*>(mem))		check(obj, deps);
	}

	void check (Ast_Deref* deref, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&deref->memory)) check(deref->memory, deps);
	}

	void check (AST_Ref* ref, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ref->memory)) check(ref->memory, deps);
	}

	void check (Ast_Attribute* attr, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&attr->exp)) check(attr->exp, deps);
	}

	void check (Ast_Type_Definition* ty, AddrDeps* deps) {
		if 		(auto obj = dynamic_cast<Ast_Struct_Type*>(ty)) 	check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Pointer_Type*>(ty))  check(obj, deps);
		else if (auto obj = dynamic_cast<Ast_Function_Type*>(ty))  		check(obj, deps);
	}

	void check (Ast_Struct_Type* ty, AddrDeps* deps) {
		for (auto attr : ty->attrs) check(attr, deps);
	}

	void check (Ast_Pointer_Type* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->base)) this->check(ty->base, deps);
	}

	void check (Ast_Function_Type* ty, AddrDeps* deps) {
		if (!deps->addIfUnresolved(&ty->retType)) this->check(ty->retType, deps);
	}
};
