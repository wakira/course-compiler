#ifndef _AST_HEADER
#define _AST_HEADER

#include <list>
#include <string>
#include <llvm/IR/Value.h>

class CGContext;

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
    virtual ~ASTNode() {};
    virtual llvm::Value* codeGen(CGContext& context) = 0; 

};

class Identifier {
public:
	std::string name;
};


class Declaration : public ASTNode
{
    virtual llvm::Value* codeGen(CGContext& context);
};

class VariableDeclaration : public Declaration {
public:
	Identifier *type;
	Identifier *name;
        virtual llvm::Value* codeGen(CGContext& context); 
};

class Definition : public Declaration
{
        virtual llvm::Value* codeGen(CGContext& context);     
};

class ArrayDefinition : public Definition {
public:
    Identifier *type;
    Identifier *name;
    int size;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class ClassDefinition : public Definition {
public:
	Identifier *className;
        Identifier *extFrom;
	ElementList *variables;
	ElementList *functions;
        virtual llvm::Value* codeGen(CGContext& context); 
};

class FunctionDefinition : public Definition {
public:
        Identifier *name;
	Identifier *retType;
	ElementList *arguments;
	ElementList *args_var;
    ElementList *variables;
	ElementList *functionBlock;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class Statement : public ASTNode {
    virtual llvm::Value* codeGen(CGContext& context); 
};
class Expression : public ASTNode {
  public:
    virtual llvm::Value* codeGen(CGContext& context); 
};
class Primary: public Expression {
  public:
    Expression *expr;
    virtual llvm::Value* codeGen(CGContext& context);
    virtual llvm::Value* codeGenRef(CGContext& context);
};
class IdentPr: public Primary {
  public:
    Identifier *name;
    virtual llvm::Value* codeGen(CGContext& context);
    virtual llvm::Value* codeGenRef(CGContext& context);    
};

class ArrayPr: public Primary {
  public:
    Primary *name;
    Expression *index;
    virtual llvm::Value* codeGen(CGContext& context);
    virtual llvm::Value* codeGenRef(CGContext& context);    
};

class NumericLiteral : public Primary {
public:
	int val;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class BinaryOperation : public Expression {
public:
    enum Ops {P, S, M, D, MOD, E, NE, L, LE, G, GE, A, O, NONE};
         /* Plus, Sub, Multi, Div, Mod, Eq, NotEq, Less, LessEq, Greater, GEq, And, Or */
	Expression *a, *b;
	Ops op;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class UnaryOperation : public Expression {
  public:
    Primary *p;
    BinaryOperation::Ops op;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class FunCall : public Primary {
  public:
    Primary *name;
    ElementList *args;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class DotOperation : public Primary {
  public:
    Primary *pr;
    Identifier *field;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class ElementList : public ASTNode {
public:
	std::list<ASTNode*> elements;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class RetStatement: public Statement {
  public:
    Expression *expr;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class AssignmentStatement : public Statement {
public:
	Primary *lhs;
	Expression *rhs;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class IOStatement: public Statement {
  public:
    enum IO {IN, OUT};
    Expression *content;
    Primary *var;
    IO op;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class IfStatement : public Statement {
  public:
    ElementList *conds;
    std::list<ElementList *> stats;
    virtual llvm::Value* codeGen(CGContext& context); 
};

class LoopStatement: public Statement {
  public:
    enum Type { WHILE, REPEAT };
    Type type;
    Expression *cond;
    ElementList *stats;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class ForeachStatement: public Statement {
  public:
    Identifier *element;
    Identifier *array;
    ElementList *stats;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class FuncStatement: public Statement {
  public:
    FunCall *call;
    virtual llvm::Value* codeGen(CGContext& context); 
};
class Program : public ASTNode {
public:
    Identifier *name;
	ElementList *definitions;
	ElementList *variableDeclarations;
	ElementList *programBlock;
    virtual llvm::Value* codeGen(CGContext& context); 
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
#endif
