#include "symbol_table.hpp"
#include "types.hpp"
namespace leviathan{

SymbolTable::SymbolTable(){
	scopeTableChain = new std::list<ScopeTable *>();
}

void SymbolTable::print(){
	for(auto scope : *scopeTableChain){
		std::cout << "--- scope ---\n";
		std::cout << scope->toString();
	}
	std::cout << "----END TABLE-----\n\n";
}

ScopeTable * SymbolTable::enterScope(){
	ScopeTable * newScope = new ScopeTable();
	scopeTableChain->push_front(newScope);
	return newScope;
}

void SymbolTable::leaveScope(){
	if (scopeTableChain->empty()){
		throw new InternalError("Attempt to pop"
			"empty symbol table");
	}
	ScopeTable * scope = scopeTableChain->front();
	scope->leave();
	scopeTableChain->pop_front();
}

void ScopeTable::leave(){
	for (auto binding : *(this->symbols)){
		SemSymbol * sym = binding.second;
		sym->surface();
	}
}

ScopeTable * SymbolTable::getCurrentScope(){
	return scopeTableChain->front();
}

bool SymbolTable::clash(std::string varName){
	bool hasClash = getCurrentScope()->clash(varName);
	return hasClash;
}

SemSymbol * SymbolTable::find(std::string varName){
	for (ScopeTable * scope : *scopeTableChain){
		SemSymbol * sym = scope->lookup(varName);
		if (sym != nullptr) { return sym; }
	}
	return nullptr;
}

bool SymbolTable::insert(SemSymbol * symbol){
	return scopeTableChain->front()->insert(symbol);
}

ScopeTable::ScopeTable(){
	symbols = new std::list<std::pair<std::string, SemSymbol *>>();
}

std::string ScopeTable::toString() const{
	std::string result = "";
	bool first = true;
	for (auto entry : *symbols){
		if (first){ first = false; }
		else { result += "  --"; }

		result += entry.second->toString();
	}
	return result;
}

bool ScopeTable::clash(std::string varName){
	SemSymbol * found = lookup(varName);
	if (found != nullptr){
		return true;
	}
	return false;
}

SemSymbol * ScopeTable::lookup(std::string nameIn){
	for( auto binding : *symbols){
		std::string symName = binding.first;
		if (symName != nameIn){
			continue;
		}
		SemSymbol * found = binding.second;
		if (found->isSunk()){
			continue;
		}
		return found;
	}
	return nullptr;
}

bool ScopeTable::insert(SemSymbol * symbol){
	std::string symName = symbol->getName();
	bool alreadyInScope = (this->lookup(symName) != NULL);
	if (alreadyInScope){
		return false;
	}
	this->symbols->push_back(std::make_pair(symName, symbol));
	return true;
}

std::string SemSymbol::toString() const{
	std::string result = "";
	result += "name: " + this->getName();
	result += "\nkind: " + kindToString(this->getKind());
	const DataType * type = this->getDataType();
	if (type == nullptr){
		result += "\ntype: NULL";
	} else {
		result += "\ntype: " + this->getDataType()->getString();
	}
	return result + "\n";
}

}
