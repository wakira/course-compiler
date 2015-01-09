#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <boost/program_options.hpp>

#include "ast.h"
#include "codegen.h"
#include "present.h"
#include "syntax.hpp"

#define PRTTOK(tok) case tok:cout<<(""#tok)<<endl;break

using namespace std;
extern Program *astRoot;
extern FILE *yyin;
extern int yyparse();
extern int yylex(void);

int main(int argc, char **argv) {
	using namespace boost::program_options;
	auto desc = options_description{"options"};
	desc.add_options()
		("lexical,l", "show lexical analysis result")
		("syntax,s", "show lexical analysis result")
		("help,h", "display help message")
		("file", value<std::string>(), "filename of source code")
		;

	auto vm = variables_map{};
	store(command_line_parser{argc, argv}
			.options(desc)
			.positional(
				positional_options_description{}
				.add("file", 1))
			.run() , vm);
	notify(vm);

	if (vm.count("help")) {
		cout << desc;
		return 0;
	}
	if (!vm.count("file")) {
		cerr << "Must specify input file" << endl;
		return 1;
	}
	string filename = vm["file"].as<std::string>();
	yyin = fopen(filename.c_str(), "r");
	if (!fopen) {
		cerr << "Cannot open file " << filename << endl;
		return 1;
	}
	if (vm.count("lexical")) {
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
		return 0;
	} else if (vm.count("syntax")) {
		yyparse();
		string ast = ast_string(astRoot);
		cout << ast << endl;
		return 0;
	}

	yyparse();
	if (!astRoot) {
		return 1;
	}
	InitializeNativeTarget();
	CGContext context;
	if (context.generateCode(astRoot)) {
		context.runCode();
	}
	return 0;
}
