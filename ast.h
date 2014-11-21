#include <list>
#include <string>
//#include <llvm/Value.h>

//class CodeGenContext;

class ASTNode;
class Identifier;
class Declaration;
class VariableDeclaration;
class Definition;
class ArrayDefinition;
class ClassDefinition;
class FunctionDefinition;
class Statement;
class Expression;
class BinaryOperation;
class ElementList;
class Program;

class ASTNode {
public:
	// Non virtual function during debug
	/*
	virtual ~ASTNode() = 0;
	virtual llvm::Value* codeGen(CodeGenContext& context) = 0; 
	*/
};

class Identifier {
public:
	std::string name;
};

class Declaration : public ASTNode {};

class VariableDeclaration : public Declaration {
public:
	Identifier *type;
	Identifier *name;
};

class Definition : public Declaration {};

class ArrayDefinition : public Definition {
public:
	
};

class ClassDefinition : public Definition {
public:
	Identifier *className;
	std::list<VariableDeclaration*> variables;
	std::list<FunctionDefinition*> functions;
};

class FunctionDefinition : public Definition {
public:
	Identifier *name;
	Identifier *retType;
	ElementList *arguments;
	ElementList *variables;
	ElementList *functionBlock;
};

class Statement : public ASTNode {};
class Expression : public ASTNode {};

class NumericLiteral : public Expression {
public:
	int val;	
};

class BinaryOperation : public Expression {
public:
	Expression *a, *b;
	int op;
};

class DotOperation : public Expression {
	
};

class ElementList : public ASTNode {
public:
	std::list<ASTNode*> elements;
};

/*
class AssignmentStatement : public Statement {
public:
	LeftValue lhs;
	Expression rhs;
};

class IfStatement : public Statement {
	// TODO

};
*/

class Program : public ASTNode {
public:
	ElementList *definitions;
	ElementList *variableDeclarations;
	ElementList *programBlock;
};

/*
class ASTBlock: public ASTNode {
public:
	std::list<ASTStatement> statements;
}

class ASTFunctionDeclaration : public ASTStatement {

};

class ASTVariableDeclaration : public ASTStatement {

};
*/
