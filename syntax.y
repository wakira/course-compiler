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
        Definition *type_definition;
	VariableDeclaration *type_variable_declaration;
	Identifier *type_identifier;
        Statement *type_statement;
        Expression *type_expression;
        Primary *type_primary;
        ClassDefinition *type_class_def;
        ArrayDefinition *type_array_def;
        FunctionDefinition *type_func_def;
    IOStatement *type_iostat;
    IfStatement *type_ifstat;
    AssignmentStatement *type_assignstat;
    BinaryOperation *type_binexpr;
    UnaryOperation *type_uexpr;
    NumericLiteral *type_intpr;
    IdentPr *type_identpr;
    FunCall *type_funcall;
    DotOperation *type_dotop;
    ArrayPr *type_arrpr;
    RetStatement *type_retstat;
    Identifier *type_ident;
    FuncStatement *type_funcstat;
}

%token PLUS MINUS MUL DIV LPAR RPAR EQ NE LT LE GT GE
/* Keywords */
%token PROGRAM VAR IS BEG SEMICOLON END ASSIGN AND OR XOR FUNCTION IF ELSE ELIF OUT IN
%token RETURN COMMA DOT LSQR RSQR ARRAY THEN OF TYPE EXT CLASS
%token INTEGER BOOLEAN
/* Literals */
%token <type_int> INT
%token <type_string> IDENTIFIER

/* Type mapping to AST */
%type <type_program> program 
%type <type_identifier> ret_decl
%type <type_variable_declaration> var_decl
%type <type_element_list> block statements var_decls defs args_decl more_args_decl func_defs else_semi_stat args_list more_args_list
%type <type_definition> def
%type <type_class_def> class_def
%type <type_array_def> array_def
%type <type_func_def> func_def
%type <type_statement> statement
%type <type_funcstat> func_statement
%type <type_assignstat> assignment
%type <type_iostat> print_statement
%type <type_retstat> ret_statement
%type <type_ifstat> if_statement elif_semi_stat elif_semi_stats
%type <type_expression> expression 
%type <type_binexpr> cond_or cond_and eq_op rel_op add_op multi_op
%type <type_uexpr> unary_op
%type <type_primary> primary 
%type <type_funcall> func_call
%type <type_dotop> field_acc
%type <type_intpr> int_pr
%type <type_arrpr> array_acc
%type <type_identpr> ident_pr
%type <type_ident> identifier
%left PLUS MINUS
%left MUL DIV

%start program

%%

program: 
	PROGRAM identifier LPAR RPAR defs IS var_decls block
	{
		$$ = new Program();
		$$->definitions = $5;
		$$->variableDeclarations = $7;
		$$->programBlock = $8;
                $$->name = $2;
		printf("Parsing of program '%s' finished successfully\n", $$->name->name.c_str());
	};

defs:
        /* epsilon */ { $$ = nullptr; }
	| def defs
        {
            $$ = new ElementList();
            $$->elements.push_back($1);
            if ($2 != nullptr) {
                $$->elements.splice($$->elements.end(), $2->elements);
            }
        };

def:
        func_def { $$ = $1; }
        | array_def { $$ = $1; }
        | class_def { $$ = $1; };
class_def:
        TYPE identifier IS CLASS var_decls func_defs END CLASS SEMICOLON
        {
            $$ = new ClassDefinition();
            $$->className = $2;
            $$->extFrom = nullptr;
            $$->variables = $5;
            $$->functions = $6;
        }
        | TYPE identifier IS CLASS EXT identifier var_decls func_defs END CLASS SEMICOLON
        {
            $$ = new ClassDefinition();
            $$->className = $2;
            $$->extFrom = $6;
            $$->variables = $7;
            $$->variables = $8;
        }

array_def:
        TYPE identifier IS ARRAY OF INT identifier SEMICOLON
        {
            $$ = new ArrayDefinition();
            $$->type = $7;
            $$->name = $2;
            $$->size = $6;
        };
func_defs:
        /* epsilon */ { $$ = nullptr; }
        | func_def func_defs
        {
            $$ = new ElementList();
            $$->elements.push_back($1);
            if ($2 != nullptr) {
                $$->elements.splice($$->elements.end(), $2->elements);
            }
        }
func_def:
	FUNCTION identifier LPAR args_decl RPAR var_decls ret_decl IS var_decls block FUNCTION identifier SEMICOLON
	{
		$$ = new FunctionDefinition();
		$$->name = $2;
		$$->arguments = $4;
		$$->variables = $6;
		$$->retType = $7;
		$$->functionBlock = $9;
	};

args_decl:
	/* epsilon */ { $$ = nullptr; }
	| ident_pr more_args_decl
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
	| COMMA ident_pr more_args_decl
	{
		$$ = new ElementList();
		$$->elements.push_back($2);
		if ($3 != nullptr) {
			$$->elements.splice($$->elements.end(), $3->elements);
		}
	};
identifier:
        IDENTIFIER
        {
            $$ = new Identifier;
            $$->name = *($1);
        };
args_list:
        /* epsilon */ { $$ = nullptr; }
        | expression more_args_list
        {
            $$ = new ElementList();
            $$->elements.push_back($1);
            if ($2 != nullptr) {
                $$->elements.splice($$->elements.end(), $2->elements);
            }
        };
more_args_list:
        /* epsilon */ { $$ = nullptr; }
        | COMMA expression more_args_list
        {
            $$ = new ElementList();
            $$->elements.push_back($2);
            if ($3 != nullptr) {
                $$->elements.splice($$->elements.end(), $3->elements);
            }
        };
ret_decl:
	/* epsilon */ { $$ = nullptr; }
	| RETURN identifier SEMICOLON
	{
		$$ = $2;
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
	VAR identifier IS identifier SEMICOLON
	{
		$$ = new VariableDeclaration();
		$$->type = $4;
		$$->name = $2;
	};

block:
        BEG statements END { $$ = $2; };

statements:
        /* epsilon */ { $$ = nullptr; }
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
        | if_statement { $$ = $1; }
        | ret_statement { $$ = $1; }
        | func_statement { $$ = $1; };
func_statement:
        func_call SEMICOLON
        {
            $$ = new FuncStatement();
            $$->call = $1;
        };
ret_statement:
        RETURN expression SEMICOLON
        {
            $$ = new RetStatement();
            $$->expr = $2;
        };
print_statement:
        OUT expression SEMICOLON
        {
            $$ = new IOStatement();
            $$->content = $2;
            $$->op = IOStatement::OUT;
        };
if_statement:
        IF expression THEN statements elif_semi_stats else_semi_stat END IF
        {
            $$ = new IfStatement();
            $$->conds.elements.push_back($2);
            $$->stats.push_back($4);
            if ($5 != nullptr) {
                $$->conds.elements.splice($$->conds.elements.end(), $5->conds.elements);
                $$->stats.splice($$->stats.end(), $5->stats);
            }
            if ($6 != nullptr) {
                $$->conds.elements.push_back(nullptr);
                $$->stats.push_back($6);
            }
        };
elif_semi_stats:
        /* epsilon */ {$$ = nullptr;}
        | elif_semi_stat elif_semi_stats
        {
            $$ = $1;
            if ($2 != nullptr) {
                $$->conds.elements.splice($$->conds.elements.end(), $2->conds.elements);
                $$->stats.splice($$->stats.end(), $2->stats);
            }
        };
elif_semi_stat:
        ELIF expression THEN statements
        {
            $$ = new IfStatement();
            $$->conds.elements.push_back($2);
            $$->stats.push_back($4);
        };
else_semi_stat:
        /* epsilon */ { $$ = nullptr; }
        | ELSE statements
        {
            $$ = $2;
        };
        

assignment:
	ident_pr ASSIGN expression SEMICOLON
	{
                $$ = new AssignmentStatement();
                $$->lhs = $1;
                $$->rhs = $3;
	}
	| field_acc ASSIGN expression SEMICOLON
	{
                $$ = new AssignmentStatement();
                $$->lhs = $1;
                $$->rhs = $3;
	}
	| array_acc ASSIGN expression SEMICOLON
	{
                $$ = new AssignmentStatement();
                $$->lhs = $1;
                $$->rhs = $3;
	};


expression:
       cond_or { $$ = $1; };
cond_or:
       cond_and
       {
            $$ = new BinaryOperation();
            $$->a = $1;
            $$->b = nullptr;
            $$->op = BinaryOperation::NONE;
       }
       | cond_and OR cond_or
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
       | eq_op AND cond_and
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
        primary
        {
            $$ = new UnaryOperation();
            $$->p = $1;
            $$->op = BinaryOperation::NONE;
        }
        | PLUS primary
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
        int_pr { $$ = $1; }
	| ident_pr { $$ = $1; }
        | array_acc { $$ = $1; }
        | field_acc { $$ = $1; }
        | func_call { $$ = $1; }
        | LPAR expression RPAR
        {
            $$ = new Primary();
            $$->expr = $2;
        }
int_pr:
        INT
        {
            $$ = new NumericLiteral();
            $$->val = $1;
        };
ident_pr:
        identifier
        {
            $$ = new IdentPr();
            $$->name = $1;
        };
func_call:
        ident_pr LPAR args_list RPAR
        {
            $$ = new FunCall();
            $$->name = $1;
            $$->args = $3;
        }
        | field_acc LPAR args_list RPAR
        {
            $$ = new FunCall();
            $$->name = $1;
            $$->args = $3;
        };
field_acc:
        primary DOT identifier
        {
            $$ = new DotOperation();
            $$->pr = $1;
            $$->field = $3;
        };
array_acc:
        identifier LSQR expression RSQR
        {
            $$ = new ArrayPr();
            $$->name = $1;
            $$->index = $3;
        };
