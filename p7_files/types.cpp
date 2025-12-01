#include <list>
#include <sstream>

#include "types.hpp"
#include "ast.hpp"

namespace leviathan{

std::string BasicType::getString() const{
	std::string res = "";
	switch(myBaseType){
	case BaseType::INT:
		res += "int";
		break;
	case BaseType::BOOL:
		res += "bool";
		break;
	case BaseType::VOID:
		res += "void";
		break;
	case BaseType::STRING:
		res += "string";
		break;
	case BaseType::FILE:
		res += "file";
		break;
	}
	return res;
}

const DataType * FileTypeNode::getType() const {
	return BasicType::FILE();
}

const DataType * BoolTypeNode::getType() const {
	return BasicType::BOOL();
}

const DataType * IntTypeNode::getType() const {
	return BasicType::INT();
}

static bool typelistMatch(const std::list<const DataType *> * first, const std::list<const DataType *> * second){
	if (first->size() != second->size()){
		return false;
	}

	auto kTypeItr = first->begin();
	auto cTypeItr = second->begin();
	while (kTypeItr != first->end()){
		auto kType = *kTypeItr;
		auto cType = *cTypeItr;
		if (kType != cType){
			return false;
		}
		++kTypeItr;
		++cTypeItr;
	}
	return true;
}

TypeList * TypeList::produce(const std::list<const DataType *> * eltTypes){
	//Use a flyweight here
	static std::list<TypeList *> knownLists;

	TypeList * exists = nullptr;
	for (TypeList * known : knownLists){
		if (typelistMatch(known->types, eltTypes)){
			exists = known;
			break;
		}
	}

	if (!exists){
		TypeList * t = new TypeList(eltTypes);
		knownLists.push_back(t);
		return t;
	} else {
		return exists;
	}
}

} //End namespace
