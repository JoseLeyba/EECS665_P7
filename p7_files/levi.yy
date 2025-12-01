%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{leviathan}
%define api.parser.class {Parser}
%define api.value.type variant
%define parse.error verbose
%output "parser.cc"
%token-table

%code requires{
	#include <list>
	#include "tokens.hpp"
	#include "ast.hpp"
	namespace leviathan {
		class Scanner;
	}

//The following definition is required when 
// we don't use the %locations directive (which we won't)
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

//End "requires" code
}

%parse-param { leviathan::Scanner &scanner }
%parse-param { leviathan::ProgramNode** root}
%code {
   // C std code for utility functions
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   // Our code for interoperation between scanner/parser
   #include "scanner.hpp"
   #include "ast.hpp"
   #include "tokens.hpp"

  //Request tokens from our scanner member, not 
  // from a global function
  #undef yylex
  #define yylex scanner.yylex
}

//%define parse.assert

/* Terminals */


%token                     END   0 "end file"
%token	<leviathan::Token *>       AND
%token	<leviathan::Token *>       ASSIGN
%token	<leviathan::Token *>       BOOL
%token	<leviathan::Token *>       COLON
%token	<leviathan::Token *>       COMMA
%token	<leviathan::Token *>       DASH
%token	<leviathan::Token *>       ELSE
%token	<leviathan::Token *>       SINK
%token	<leviathan::Token *>       EQUALS
%token	<leviathan::Token *>       FALSE
%token	<leviathan::Token *>       FILE
%token	<leviathan::Token *>       OUTPUT
%token	<leviathan::Token *>       GREATER
%token	<leviathan::Token *>       GREATEREQ
%token	<leviathan::IDToken *>     ID
%token	<leviathan::Token *>       IF
%token	<leviathan::Token *>       INT
%token	<leviathan::IntLitToken *> INTLITERAL
%token	<leviathan::Token *>       IMMUTABLE
%token	<leviathan::Token *>       LCURLY
%token	<leviathan::Token *>       LESS
%token	<leviathan::Token *>       LESSEQ
%token	<leviathan::Token *>       LPAREN
%token	<leviathan::Token *>       NOT
%token	<leviathan::Token *>       NOTEQUALS
%token	<leviathan::Token *>       OR
%token	<leviathan::Token *>       CROSS
%token	<leviathan::Token *>       POSTDEC
%token	<leviathan::Token *>       POSTINC
%token	<leviathan::Token *>       RETURN
%token	<leviathan::Token *>       RCURLY
%token	<leviathan::Token *>       RPAREN
%token	<leviathan::Token *>       SEMICOL
%token	<leviathan::Token *>       SLASH
%token	<leviathan::Token *>       STAR
%token	<leviathan::StrToken *>    STRINGLITERAL
%token	<leviathan::Token *>       INPUT
%token	<leviathan::Token *>       LBRACKET
%token	<leviathan::Token *>       RBRACKET
%token	<leviathan::Token *>       THRASH
%token	<leviathan::Token *>       TRUE
%token	<leviathan::Token *>       VOID
%token	<leviathan::Token *>       WHILE

%type <leviathan::ProgramNode *> program
%type <std::list<leviathan::DeclNode *> *> globals
%type <leviathan::DeclNode *> decl
%type <leviathan::VarDeclNode *> varDecl
%type <leviathan::FnDeclNode *> fnDecl
%type <leviathan::ExpNode *> term
%type <leviathan::ExpNode *> literal
%type <std::list<leviathan::ExpNode *> *> litList
%type <leviathan::ExpNode *> exp
%type <std::list<leviathan::ExpNode *> *> actualsList
%type <leviathan::InitializerNode *> initializer
%type <leviathan::CallExpNode *> callExp

%type <leviathan::IDNode *> name
%type <leviathan::LocNode *> loc
%type <leviathan::StmtNode *> stmt
%type <leviathan::StmtNode *> blockStmt
%type <std::list<leviathan::StmtNode *> *> stmtList
%type <leviathan::TypeNode *> type
%type <leviathan::TypeNode *> dataType
%type <leviathan::FormalDeclNode *> formalDecl
%type <std::list<leviathan::FormalDeclNode *> *> maybeFormals
%type <std::list<leviathan::FormalDeclNode *> *> formalList

%type <leviathan::TypeNode *> primType

/* NOTE: Make sure to add precedence and associativity 
 * declarations
 */
%right ASSIGN
%left OR
%left AND
%nonassoc LESS GREATER LESSEQ GREATEREQ EQUALS NOTEQUALS
%left DASH CROSS
%left STAR SLASH
%left NOT 

%%

program		: globals
		  {
		  $$ = new ProgramNode($1);
		  *root = $$;
		  }

globals		: globals decl
		  {
		  $$ = $1;
		  DeclNode * declNode = $2;
		  $$->push_back(declNode);
		  }
		| /* epsilon */
		  {
		  $$ = new std::list<DeclNode *>();
		  }

decl		: varDecl SEMICOL
		  {
		  $$ = $1;
		  }
		| fnDecl
		  {
		  $$ = $1;
		  }

varDecl		: name COLON type
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new VarDeclNode(p,$1, $3, nullptr);
		  }
		| name COLON type ASSIGN initializer
		  {
		  Position * p = new Position($1->pos(), $5->pos());
		  $$ = new VarDeclNode(p,$1, $3, $5);
		  }

type		: IMMUTABLE dataType
		  {
		  Position * p = new Position($1->pos(), $2->pos());
		  throw new RemovedError(p->span());
		  }
		| dataType
		  {
		  $$ = $1;
		  }

dataType	: primType LBRACKET INTLITERAL RBRACKET
		  {
		  Position * p = new Position($1->pos(), $4->pos());
		  throw new RemovedError(p->span());
		  }
		| primType
		  {
		  $$ = $1;
		  }

primType	: INT
		  {
		  $$ = new IntTypeNode($1->pos());
		  }
		| BOOL
		  {
		  $$ = new BoolTypeNode($1->pos());
		  }
		| FILE
		  {
		  $$ = new FileTypeNode($1->pos());
		  }
		| VOID
		  {
		  $$ = new VoidTypeNode($1->pos());
		  }

fnDecl 		: name COLON LPAREN maybeFormals RPAREN type LCURLY stmtList RCURLY
		  {
		  auto pos = new Position($1->pos(), $9->pos());
		  $$ = new FnDeclNode(pos, $1, $4, $6, $8);
		  }

maybeFormals	: /* epsilon */
		  {
		  $$ = new std::list<FormalDeclNode *>();
		  }
		| formalList
		  {
		  $$ = $1;
		  }

formalList	: formalDecl
		  {
		  $$ = new std::list<FormalDeclNode *>();
		  $$->push_back($1);
		  }
		| formalList COMMA formalDecl
		  {
		  $$ = $1;
		  $$->push_back($3);
		  }

formalDecl	: name COLON type
		  {
		  auto pos = new Position($1->pos(), $3->pos());
		  $$ = new FormalDeclNode(pos, $1, $3, nullptr);
		  }
		| name COLON type ASSIGN initializer
		  {
		  auto pos = new Position($1->pos(), $5->pos());
		  $$ = new FormalDeclNode(pos, $1, $3, $5);
		  }

stmtList	: /* epsilon */
		  {
		  $$ = new std::list<StmtNode *>();
		  }
		| stmtList stmt SEMICOL
		  {
		  $$ = $1;
		  $$->push_back($2);
		  }
		| stmtList blockStmt
		  {
		  $$ = $1;
		  $$->push_back($2);
		  }

blockStmt	: WHILE LPAREN exp RPAREN LCURLY stmtList RCURLY
		  {
		  const Position * p = new Position($1->pos(), $7->pos());
		  $$ = new WhileStmtNode(p, $3, $6);
		  }
		| IF LPAREN exp RPAREN LCURLY stmtList RCURLY
		  {
		  const Position * p = new Position($1->pos(), $7->pos());
		  $$ = new IfStmtNode(p, $3, $6);
		  }
		| IF LPAREN exp RPAREN LCURLY stmtList RCURLY ELSE LCURLY stmtList RCURLY
		  {
		  const Position * p = new Position($1->pos(), $11->pos());
		  $$ = new IfElseStmtNode(p, $3, $6, $10);
		  }

stmt		: varDecl
		  {
		  $$ = $1;
		  }
		| loc ASSIGN exp
		  {
		  const auto p = new Position($1->pos(), $3->pos());
		  $$ = new AssignStmtNode(p, $1, $3); 
		  }
		| callExp
		  {
		  $$ = new CallStmtNode($1->pos(), $1);
		  }
		| loc POSTDEC
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new PostDecStmtNode(p, $1);
		  }
		| loc POSTINC
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new PostIncStmtNode(p, $1);
		  }
		| loc OUTPUT exp
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new WriteStmtNode(p, $1, $3);
		  }
		| loc INPUT loc
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new ReadStmtNode(p, $1, $3);
		  }
		| SINK name
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new SinkStmtNode(p, $2);
		  }
		| RETURN exp
		  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new ReturnStmtNode(p, $2);
		  }
		| RETURN
		  {
		  const Position * p = $1->pos();
		  $$ = new ReturnStmtNode(p, nullptr);
		  }

exp		: exp DASH exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new MinusNode(p, $1, $3);
		  }
		| exp CROSS exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new PlusNode(p, $1, $3);
		  }
		| exp STAR exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new TimesNode(p, $1, $3);
		  }
		| exp SLASH exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new DivideNode(p, $1, $3);
		  }
		| exp AND exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new AndNode(p, $1, $3);
		  }
		| exp OR exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new OrNode(p, $1, $3);
		  }
		| exp EQUALS exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new EqualsNode(p, $1, $3);
		  }
		| exp NOTEQUALS exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new NotEqualsNode(p, $1, $3);
		  }
		| exp GREATER exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new GreaterNode(p, $1, $3);
		  }
		| exp GREATEREQ exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new GreaterEqNode(p, $1, $3);
		  }
		| exp LESS exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new LessNode(p, $1, $3);
		  }
		| exp LESSEQ exp
	  	  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  $$ = new LessEqNode(p, $1, $3);
		  }
		| NOT exp
	  	  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new NotNode(p, $2);
		  }
		| DASH term
	  	  {
		  const Position * p = new Position($1->pos(), $2->pos());
		  $$ = new NegNode(p, $2);
		  }
		| term
	  	  { $$ = $1; }


callExp		: loc LPAREN RPAREN
		  {
		  const Position * p = new Position($1->pos(), $3->pos());
		  std::list<ExpNode *> * noargs =
		    new std::list<ExpNode *>();
		  $$ = new CallExpNode(p, $1, noargs);
		  }
		| loc LPAREN actualsList RPAREN
		  {
		  const Position * p = new Position($1->pos(), $4->pos());
		  $$ = new CallExpNode(p, $1, $3);
		  }

actualsList	: exp
		  {
		  std::list<ExpNode *> * list =
		    new std::list<ExpNode *>();
		  list->push_back($1);
		  $$ = list;
		  }
		| actualsList COMMA exp
		  {
		  $$ = $1;
		  $$->push_back($3);
		  }

term 		: loc
		  { $$ = $1; }
		| literal
		  { $$ = $1; }
		| THRASH 
		  { $$ = new ThrashNode($1->pos()); }
		| LPAREN exp RPAREN
		  { $$ = $2; }
		| callExp
		  { $$ = $1; } 

initializer	: literal
		  { 
		  std::list<ExpNode *> * nodes;
		  nodes = new std::list<ExpNode *>();
		  nodes->push_back($1);
		  $$ = new InitializerNode($1->pos(), nodes, false);
		  } 
		| LBRACKET litList RBRACKET
		  {
		  Position * pos = new Position($1->pos(), $3->pos());
		  $$ = new InitializerNode(pos, $2, true); 
		  } 

litList		: literal
		  {
		  std::list<ExpNode *> * nodes;
		  nodes = new std::list<ExpNode *>();
		  nodes->push_back($1);
		  $$ = nodes;
		  } 
		| literal COMMA litList
		  { 
		  $$ = $3;
		  $$->push_front($1);
		  } 

literal		: TRUE
		  { $$ = new TrueNode($1->pos()); }
		| FALSE
		  { $$ = new FalseNode($1->pos()); }
		| INTLITERAL
		  { $$ = new IntLitNode($1->pos(), $1->num()); }
		| STRINGLITERAL
		  { $$ = new StrLitNode($1->pos(), $1->str()); }

loc		: name
		  {
		  $$ = $1;
		  }
		| loc LBRACKET exp RBRACKET
		  {
		  Position * p = new Position($1->pos(), $4->pos());
		  throw new RemovedError(p->span());
		  }

name		: ID
		  {
		  $$ = new IDNode($1->pos(), $1->value());
		  }
	
%%

void leviathan::Parser::error(const std::string& msg){
	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}
