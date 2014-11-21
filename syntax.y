%{
#include "ast.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

Program *astRoot;

extern int yylex();
void yyerror(const char *s) { printf("ERROR: %s\n", s); }
%}

%union {
	int type_int;
	std::string *type_string;

	Program *type_program;
	ElementList *type_element_list;
	VariableDeclaration *type_variable_declaration;
	FunctionDeclaration *type_function_declaration;
	Identifier *type_identifier;
}

%token PLUS MINUS MUL DIV LPAR RPAR EQ NE LT LE GT GE
/* Keywords */
%token PROGRAM VAR IS BEG SEMICOLON END ASSIGN AND OR XOR PRINT FUNCTION
%token RETURN COMMA
%token INTEGER BOOLEAN
/* Literals */
%token <type_int> INT
%token <type_string> IDENTIFIER

/* Type mapping to AST */
%type <type_program> program 
%type <type_identifier> ret_decl
%type <type_variable_declaration> var_decl
%type <type_function_declaration> func_def
%type <type_element_list> block statements var_decls defs args_decl more_args_decl

%left PLUS MINUS
%left MUL DIV

%start program

%%

program: 
	PROGRAM IDENTIFIER LPAR RPAR defs IS var_decls block
	{
		$$ = new Program();
		$$->definitions = $5;
		$$->variableDeclarations = $7;
		$$->programBlock = $8;
		printf("Parsing of program %s finished successfully\n", ($2)->c_str());
	};

defs:
	/* epsilon */ {}
	| def defs {};

def:
	func_def {}
	| type_def {}
	| array_def {}
	| class_def {};

func_def:
	FUNCTION IDENTIFIER LPAR args_decl RPAR var_decls ret_decl IS block FUNCTION IDENTIFIER SEMICOLON
	{
		$$ = new FunctionDeclaration();
		$$->name = $2;
		$$->arguments = $4
		$$->variables = $6;
		$$->retType = $7;
		$$->functionBlock = $9;
	};

args_decl:
	/* epsilon */ { $$ = nullptr; }
	| IDENTIFIER more_args_decl
	{
		$$ = new ElementList();
		$$->elements.push_back($1);
		if ($2 != nullptr) {
			$$->elements.splice($$->elements.end(), $2->elements);
		}
	}
	;

more_args_decl:
	/* epsilon */ { $$ = nullptr; }
	| COMMA IDENTIFIER more_args_decl
	{
		$$ = new ElementList();
		$$->elements.push_back($2);
		if ($3 != nullptr) {
			$$->elements.splice($$->elements.end(), $3->elements);
		}
	};

ret_decl:
	/* epsilon */ { $$ = nullptr; }
	| RETURN IDENTIFIER SEMICOLON
	{
		$$ = new Identifier();
		$$->name = *($2);
	};

var_decls:
	/* epsilon */ { $$ = nullptr; }
	| var_decl var_decls
	{
		$$ = new ElementList();
		$$->elements.push_back($1);
		if ($2 != nullptr) {
			$$->elements.splice($$->elements.end(), $2->elements);
		}
	};

var_decl:
	VAR IDENTIFIER IS IDENTIFIER SEMICOLON
	{
		$$ = new VariableDeclaration();
		$$->type = new Identifier();
		$$->type->name = *($4);
		$$->name = new Identifier();
		$$->name->name = *($2);
	};

block:
	BEG statements END { $$ = $2 };

statements:
	statement 
	{
		$$ = new ElementList();
		$$->elements.push_back($1);
	}
	| statement statements 
	{
		$$ = new ElementList();
		$$->elements.push_back($1);
		if ($2 != nullptr) {
			$$->elements.splice($$->elements.end(), $2->elements);
		}
	};

statement:
	assignment 
	{
	}
	| print_statement {}
	| if_statement {}
	| 

assignment:
	IDENTIFIER ASSIGN expression SEMICOLON
	{
	}
	;

expression:
	LPAR expression RPAR { $$ = $2 }
	| INT
	| IDENTIFIER
	| expr binary_op expr
