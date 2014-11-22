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
    Identifier *type;
    Identifier *name;
    int size;
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
class Primary: public Expression {};
class IdentPr: public Primary {
    std::string name;
}

class ArrayPr: public Primary {
    std::string name;
    Expression *index;
};

class NumericLiteral : public Expression {
public:
	int val;	
};
class BinaryOperation : public Expression {
public:
    enum Ops {P, S, M, D, E, NE, L, LE, G, GE, A, O, NONE};
         /* Plus, Sub, Multi, Div, Eq, NotEq, Less, LessEq, Greater, GEq, And, Or */
	Expression *a, *b;
	Ops op;
};

class UnaryOperation : public Expression {
  public:
    Primary *p;
    Ops op;
};

class FunCall : public Primary {
  public:
    std::string name;
    ElementList *args;
};

class DotOperation : public Primary {
    Primary *pr;
    Identifier *field;
};

class ElementList : public ASTNode {
public:
	std::list<ASTNode*> elements;
};


class AssignmentStatement : public Statement {
public:
	Identifier *lhs;
	Expression *rhs;
};
class IOStatement: public Statement {
  public:
    enum IO {IN, OUT};
    Expression *content;
    IO op;
};
class IfStatement : public Statement {
  public:
    ElementList conds;
    std::list<ElementList *> stats;    
};

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
