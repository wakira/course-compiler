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
array_def:
        TYPE IDENTIFIER IS ARRAY OF INT IDENTIFIER
        {
            $$ = new ArrayDefinition();
            $$->type = $7;
            $$->name = $2;
            $$->size = $6;
        };
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
        /* episilon */ { $$ = nullptr; }
        | statement 
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
        assignment { $$ = $1; }
        | print_statement { $$ = $1; }
        | if_statement { $$ = $1; };
print_statement:
        OUT expression
        {
            $$ = new IOStatement();
            $$->content = $2;
            $$->op = OUT;
        };
if_statement:
        IF expression THEN statements elif_semi_stats else_semi_stat
        {
            $$ = new IfStatement();
            $$->conds.push_back($2);
            $$->stats.push_back($4);
            if ($5 != nullptr) {
                $$->conds.splice($$->conds.end(), $5->conds);
                $$->stats.splice($$->stats.end(), $5->stats);
            }
            if ($6 != nullptr) {
                $$->conds.push_back(nullptr);
                $$->stats.push_back($6);
            }
        };
elif_seme_stats:
        /* episilon */ {$$ = nullptr;}
        | elif_semi_stat { $$ = $1; }
        | elif_semi_stat elif_semi_stats
        {
            if ($2 != nullptr) {
                $$ = $2;
                $$->conds.push_front($1->conds[0]);
                $$->stats.push_front($1->stats[0]);
            } else {
                $$ = $1;
            }
        };
elif_semi_stat:
        /* episilon */ { $$ = nullptr; }
        | ELIF expression THEN statements
        {
            $$ = new IfStatement();
            $$->conds.push_back($2);
            $$->stats.push_back($4);
        };
else_semi_stat:
        ELSE statements
        {
            $$ = $2;
        };
        

assignment:
	IDENTIFIER ASSIGN expression SEMICOLON
	{
                $$ = new AssignmentStatement();
                $$->lhs = new Identifier();
                $$->lhs->name = $1;
                $$->rhs = $3;
	}
	;

expression:
       cond_or { $$ = $1 };
cond_or:
       cond_and
       {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
       }
       | cond_and EQ cond_or
       {
           $$ = new BinaryOperation();
           $$->a = $1;
           $$->b = $3;
           $$->op = BinaryOperation::O;
       };
cond_and:
       eq_op
       {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
       }
       | eq_op EQ cond_and
       {
           $$ = new BinaryOperation();
           $$->a = $1;
           $$->b = $3;
           $$->op = BinaryOperation::A;
       };
eq_op:
       rel_op
       {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
       }
       | rel_op EQ eq_op
       {
           $$ = new BinaryOperation();
           $$->a = $1;
           $$->b = $3;
           $$->op = BinaryOperation::E;
       }
       | rel_op NE eq_op
       {
           $$ = new BinaryOperation();
           $$->a = $1;
           $$->b = $3;
           $$->op = BinaryOperation::NE;
       };
rel_op:
        add_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
        }
        | add_op LT rel_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::L;
        }
        | add_op LE rel_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::LE;
        }
        | add_op GT rel_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::G;
        }
        | add_op GE rel_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::GE;
        };
add_op:
        multi_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
        }
        | multi_op PLUS add_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::P;
        }
        | multi_op MINUS add_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::S;
        };

multi_op:
        unary_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
        }
        | unary_op MUL multi_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::M;
        }
        | unary_op DIV multi_op
        {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = $3;
            $$->op = BinaryOperation::D;
        };
unary_op:
        PLUS primary
        {
            $$ = new UnaryOperation();
            $$->p = $2;
            $$->op = BinaryOperation::P;
        }
        | MINUS primary
        {
            $$ = new UnaryOperation();
            $$->p = $2;
            $$->op = BinaryOperation::S;
        };
primary:
        INT
        {
            $$ = new NumericLiteral();
            $$->val = $1;
        }
	| IDENTIFIER
        {
            $$ = new IdentPr();
            $$->name = $1;
        }
        | array_acc { $$ = $1; }
        | field_acc { $$ = $1; }
        | func_call { $$ = $1; };
func_call:
        IDENTIFIER LPAR args_decl RPAR
        {
            $$ = new FunCall();
            $$->name = $1;
            $$->args = $3;
        };
field_acc:
        primary DOT IDENTIFIER
        {
            $$ = new DotOperation();
            $$->pr = $1;
            $$->field = new Identifier();
            $$->field->name = $3;
        };
array_acc:
        IDENTIFIER LSQR expression RSQR
        {
            $$ = new ArrayPr();
            $$->name = $1;
            $$->index = $3;
        };
