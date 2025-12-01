#ifndef LEVI_NAME_ANALYSIS
#define LEVI_NAME_ANALYSIS

#include "ast.hpp"

namespace leviathan{

class NameAnalysis{
public:
	static NameAnalysis * build(ProgramNode * astIn){
		NameAnalysis * nameAnalysis = new NameAnalysis();
		SymbolTable * symTab = new SymbolTable();
		symTab->enterScope();

		symTab->insert(nameAnalysis->outSym);
		symTab->insert(nameAnalysis->inSym);

		bool res = astIn->nameAnalysis(symTab);
		symTab->leaveScope();
		delete symTab;
		if (!res){ return nullptr; }


		nameAnalysis->ast = astIn;
		return nameAnalysis;
	}
	ProgramNode * ast;
	VarSymbol * outSym = nullptr;
	VarSymbol * inSym = nullptr;

private:
	NameAnalysis(){
		const DataType * dataType = BasicType::FILE();
		outSym = new VarSymbol("out", dataType);
		inSym = new VarSymbol("in", dataType);
	}
};

}

#endif
