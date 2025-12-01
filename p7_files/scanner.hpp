#ifndef __LEVIATHAN_SCANNER_HPP__
#define __LEVIATHAN_SCANNER_HPP__ 1

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "frontend.hh"
#include "errors.hpp"

using TokenKind = leviathan::Parser::token;

namespace leviathan {

class Scanner : public yyFlexLexer{
public:
   
   Scanner(std::istream *in) : yyFlexLexer(in)
   {
	lineNum = 1;
	colNum = 1;
   };
   virtual ~Scanner() {
   };

   //get rid of override virtual function warning
   using FlexLexer::yylex;

   // YY_DECL defined in the flex leviathan.l
   virtual int yylex( leviathan::Parser::semantic_type * const lval);

   int makeBareToken(int tagIn){
	size_t len = static_cast<size_t>(yyleng);
	Position * pos = new Position(
	  this->lineNum, this->colNum,
	  this->lineNum, this->colNum+len);
        this->yylval->emplace<leviathan::Token *>(new Token(pos, tagIn));
        colNum += len;
        return tagIn;
   }

   void errIllegal(Position * pos, std::string match){
	leviathan::Report::fatal(pos, 
	"Illegal character " + match);
   }

   void errStrEsc(Position * pos){
	leviathan::Report::fatal(pos, 
	"String literal with bad escape sequence detected");
   }

   void errStrUnterm(Position * pos){
	leviathan::Report::fatal(pos,
	"Unterminated string literal detected");
   }

   void errStrEscAndUnterm(Position * pos){
	leviathan::Report::fatal(pos, 
	"Unterminated string literal with bad escape sequence detected");
   }

   void errIntOverflow(Position * pos){
	leviathan::Report::fatal(pos, "Integer literal overflow");
   }

   static std::string tokenKindString(int tokenKind);

   void outputTokens(std::ostream& outstream);

private:
   leviathan::Parser::semantic_type *yylval = nullptr;
   size_t lineNum;
   size_t colNum;
};

} /* end namespace */

#endif /* END __LEVIATHAN_SCANNER_HPP__ */
