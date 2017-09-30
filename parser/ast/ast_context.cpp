#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

struct ASTExpression;

struct ASTContext {
	ASTContext* parent = nullptr;
	map<string, ASTExpression*> variables;
	map<string, vector<ASTExpression**>> unresolved;

	ASTContext (ASTContext* parent = nullptr) {
		this->parent = parent;
		if (parent == nullptr) {
			this->add("void", ASTPrimitiveType::_void);
			this->add("i32", ASTPrimitiveType::_i32);
			this->add("int", ASTPrimitiveType::_i32);
		}
	}

	void add (string name, ASTExpression* val) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			this->variables[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
		}
	}

	ASTExpression* get (string name) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			if (this->parent == nullptr)  return nullptr;
			else return this->parent->get(name);
		} else return this->variables[name];
	}

	void addUnresolved (string name, ASTExpression** addr) {
		auto it = unresolved.find(name);
		if (it == unresolved.end()) {
			vector<ASTExpression**> toEdit { addr };
			unresolved[name] = toEdit;
		} else unresolved[name].push_back(addr);
	}

	bool resolve () {
		bool result = true;
		cout << "Resolving names (" << unresolved.size() << ")\n";
		for (auto it = unresolved.begin(); it != unresolved.end();) {
			cout << "\t" << it->first << " -> " << it->second.size() << " ";
			auto value = this->get(it->first);
			if (value != nullptr) {
				cout << "V";
				for (auto const &addrs : it->second)
					(*addrs) = value;
				it = unresolved.erase(it);
			} else {
				result = false;
				it++;
			}
			cout << "\n";
		}
		return result;
	}

	ASTContext* push () {
		return new ASTContext(this);
	}

	ASTContext* pop () {
		this->resolve();
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
