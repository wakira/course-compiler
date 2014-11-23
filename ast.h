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
        Identifier *extFrom;
	ElementList *variables;
	ElementList *functions;
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
class Primary: public Expression {
  public:
    Expression *expr;
};
class IdentPr: public Primary {
  public:
    Identifier *name;
};

class ArrayPr: public Primary {
  public:
    Identifier *name;
    Expression *index;
};

class NumericLiteral : public Primary {
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
    BinaryOperation::Ops op;
};

class FunCall : public Primary {
  public:
    Primary *name;
    ElementList *args;
};

class DotOperation : public Primary {
  public:
    Primary *pr;
    Identifier *field;
};

class ElementList : public ASTNode {
public:
	std::list<ASTNode*> elements;
};

class RetStatement: public Statement {
  public:
    Expression *expr;
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
class FuncStatement: public Statement {
  public:
    FunCall *call;
};
class Program : public ASTNode {
public:
    Identifier *name;
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
