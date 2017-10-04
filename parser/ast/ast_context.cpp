#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

struct ASTExpression;

struct ASTContext {
	ASTContext* parent = nullptr;
	map<string, ASTExpression*> variables;
	map<string, vector<void*>> unresolved;

	ASTContext (ASTContext* parent = nullptr) {
		this->parent = parent;
		if (parent == nullptr) {
			this->add("void", ASTPrimitiveType::_void);
			this->add("i32", ASTPrimitiveType::_i32);
			//this->add("int", ASTPrimitiveType::_i32);
		}
	}

	void add (string name, ASTExpression* val) {
		this->resolve(name, val);
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			this->variables[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
			exit(1);
		}
	}

	ASTExpression* get (string name) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			if (this->parent == nullptr)  return nullptr;
			else return this->parent->get(name);
		} else return this->variables[name];
	}

	void addUnresolved (string name, void* addr) {
		auto it = unresolved.find(name);
		if (it == unresolved.end()) {
			vector<void*> toEdit { addr };
			unresolved[name] = toEdit;
		} else unresolved[name].push_back(addr);
	}

	bool resolve (string name, ASTExpression* value) {
		if (dynamic_cast<ASTFunction*>(value)
			|| dynamic_cast<ASTType*>(value)) {
			auto it = this->unresolved.find(name);
			if (it != this->unresolved.end()) {
				for (auto addrs : it->second)
					memcpy(addrs, &value, sizeof(ASTExpression*));
				this->unresolved.erase(it);
				return true;
			} else return false;
		} else return false;
	}

	ASTContext* push () {
		return new ASTContext(this);
	}

	ASTContext* pop () {
		if (this->parent != nullptr) {
			auto pUnr = &this->parent->unresolved;
			for (auto const &it : unresolved) {
				auto entry = pUnr->find(it.first);
				if (entry == pUnr->end()) {
					(*pUnr)[it.first] = it.second;
				} else {
					for (auto const &ref : it.second)
						(*pUnr)[it.first].push_back(ref);
				}
			}
		}
		return this->parent;
	}
};
