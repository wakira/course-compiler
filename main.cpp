#include <iostream>
#include "ast.h"
#include "present.h"

using namespace std;
extern Program *astRoot;
extern int yyparse();

int main(int argc, char **argv) {
	yyparse();
        string ast = ast_string(astRoot);
        cout << ast << endl;
	return 0;
}
