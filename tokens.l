%{
#include "ast.h"
#include <string>
#include <cstdlib>
#include "syntax.hpp"
extern "C" int yywrap() {  }
%}

%%

[ \t\n]	;

"program"				{ return PROGRAM; }
"var"					{ return VAR; }
"type"                                  { return TYPE; }
"is"					{ return IS; }
"of"                                    { return OF; }
"array"                                 { return ARRAY; }
"begin"					{ return BEG; }
"end"					{ return END; }
"and"					{ return AND; }
"or"					{ return OR; }
"xor"					{ return XOR; }
"if"                                    { return IF; }
"then"                                  { return THEN; }
"else"                                  { return ELSE; }
"elif"                                  { return ELIF; }
"print"					{ return PRINT; }
"integer"				{ return INTEGER; }
"boolean"				{ return BOOLEAN; }
"function"				{ return FUNCTION; }
"return"				{ return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*	{
							yylval.type_string = new std::string(yytext, yyleng);
							return IDENTIFIER;
						}
[0-9]+					{
							yylval.type_int = atoi(yytext);
							return INT;
						}
"+"						{ return PLUS; }
"-"						{ return MINUS; }
"*"						{ return MUL; }
"/"						{ return DIV; }
"("						{ return LPAR; }	
")"						{ return RPAR; }	
"=="					{ return EQ; }
"!="					{ return NE; }
"<"						{ return LT; }
"<="					{ return LE; }
">"						{ return GT; }
">="					{ return GE; }
";"						{ return SEMICOLON; }
","						{ return COMMA; }
":="					{ return ASSIGN; }

%%
