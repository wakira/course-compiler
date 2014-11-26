#include <iostream>
#include <cstring>
#include "ast.h"
#include "present.h"
#include "syntax.hpp"

#define PRTTOK(tok) case tok:cout<<(""#tok)<<endl;break

using namespace std;
extern Program *astRoot;
extern int yyparse();
extern int yylex(void);

int main(int argc, char **argv) {
	if (argc == 2) {
		if (strcmp(argv[1], "l") == 0) {
			int type;
			while ((type = yylex())) {
				switch (type) {
				PRTTOK(PLUS);
				PRTTOK(MINUS);
				PRTTOK(MUL);
				PRTTOK(DIV);
				PRTTOK(LPAR);
				PRTTOK(RPAR);
				PRTTOK(EQ);
				PRTTOK(NE);
				PRTTOK(LT);
				PRTTOK(LE);
				PRTTOK(GT);
				PRTTOK(GE);
				PRTTOK(MOD);
				PRTTOK(PROGRAM);
				PRTTOK(VAR);
				PRTTOK(IS);
				PRTTOK(BEG);
				PRTTOK(SEMICOLON);
				PRTTOK(END);
				PRTTOK(ASSIGN);
				PRTTOK(AND);
				PRTTOK(OR);
				PRTTOK(XOR);
				PRTTOK(FUNCTION);
				PRTTOK(IF);
				PRTTOK(ELSE);
				PRTTOK(ELIF);
				PRTTOK(OUTPUT);
				PRTTOK(INPUT);
				PRTTOK(RETURN);
				PRTTOK(COMMA);
				PRTTOK(DOT);
				PRTTOK(LSQR);
				PRTTOK(RSQR);
				PRTTOK(ARRAY);
				PRTTOK(THEN);
				PRTTOK(OF);
				PRTTOK(TYPE);
				PRTTOK(EXT);
				PRTTOK(CLASS);
				PRTTOK(WHILE);
				PRTTOK(DO);
				PRTTOK(REPEAT);
				PRTTOK(UNTIL);
				PRTTOK(FOREACH);
				PRTTOK(IN);
				PRTTOK(INT);
				PRTTOK(IDENTIFIER);
				}
			}

		} else if (strcmp(argv[1], "s") == 0) {
			yyparse();
			string ast = ast_string(astRoot);
			cout << ast << endl;
		}
	}
	return 0;
}
