#include <iostream>
#include "ast.h"

using namespace std;
extern Program *astRoot;
extern int yyparse();

int main(int argc, char **argv) {
	yyparse();
	return 0;
}
