#include <fstream>
#include "scanner.hpp"

using namespace leviathan;

using TokenKind = leviathan::Parser::token;
using Lexeme = leviathan::Parser::semantic_type;

void Scanner::outputTokens(std::ostream& outstream){
	Lexeme lex;
	Token * t = lex.as<Token *>();
	int tokenKind;
	while(true){
		tokenKind = this->yylex(&lex);
		if (tokenKind == TokenKind::END){
			outstream << "EOF" 
			  << " [" << this->lineNum 
			  << "," << this->colNum << "]"
			  << std::endl;
			return;
		} else {
			outstream << lex.as<Token *>()->toString()
			//outstream << lex.tokenValue->toString()
			  << std::endl;
		}
	}
}
