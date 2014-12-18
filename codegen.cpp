#include <iostream>
#include <vector>

#include "ast.h"
#include "codegen.h"

using std::cout;
using std::endl;
using std::vector;

CGBlock::CGBlock(BasicBlock *_block, Value *_value):
        block(block), value(vale) {}

CGContext::CGContext():
        module(new Module("main", getGlobalContext())) {}

Local &CGContext::locals()
{
    return blocks.top()->locals;
}

BasicBlock *CGContext::currentBlock()
{
    return blocks.top()->block;
}

void CGContext::pushBlock(BasicBlock *block)
{
    blocks.push(new CGBlock(block, NULL));
}

void CGContext::popBlock()
{
    CGBlock *top = blocks.top();
    blocks.pop();
    delete top;
}

void CGContext::setCurrentRetValue(Value *value)
{
    blocks.top()->retValue = value;
}

Value *CGContext::getCurrentRetValue()
{
    return blocks.top()->retValue;
}

void CGContext::generateCode(ASTNode *root)
{
    cout << "Generating binary..." << endl;
    vector<Type *> argTypes;
    FunctionType *ftype =
            FunctionType::get(Type::getVoidTy(getGlobalContext()),
                              makeArrayRef(argTypes),
                              false);
    mainFunction = Function::Create(ftype,
                                    GlobalValue::InternalLinkage,
                                    "main",
                                    module);
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(),
                                            "entry",
                                            mainFunction,
                                            0);

    pushBlock(bblock);
    root->codeGen(*this);
    ReturnInst::Create(getGlobalContext(), bblock);
    popBlock();

    cout << "Finished generation." << endl;
    PassManager pm;
    pm.add(createPrintModulePass(outs()));
    pm.run(*module);
}

GenericValue CGContext::runCode()
{
    cout << "running..." << endl << endl;
    ExecutionEngine *ee = EngineBuilder(module).create();
    vector<GenericValue> noargs;
    GenericValue v = ee->runFunction(mainFunction, noargs);
    cout << endl << "done" << endl;
}

#define CG_FUN(nodeType) Value *nodeType::codeGen(CGContext& context)

CG_FUN(Declaration)
{
}

CG_FUN(VariableDeclaration)
{
}

CG_FUN(Definition)
{
}

CG_FUN(ArrayDefinition)
{
}

CG_FUN(ClassDefinition)
{
}

CG_FUN(FunctionDefinition)
{
}

CG_FUN(Statement)
{
}

CG_FUN(Expression)
{
}

CG_FUN(Primary)
{
}

CG_FUN(IdentPr)
{
    string ident = name->name;
    cout << "Generating identifier: " << ident << endl;
    if (context.locals().find(ident) == context.locals().end()) {
        cerr << "Err: Undeclared variable: " << ident << endl;
        return NULL;
    }
    return LoadInst(context.locals()[name],
                    "",
                    false,
                    context.currentBlock());
}

CG_FUN(ArrayPr)
{
}

CG_FUN(NumericalLiteral)
{
    cout << "Generating number: " << val << endl;
    return ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
                            val,
                            true);
}

CG_FUN(BinaryOperation)
{
    cout << "Generating binary operation: " << op << endl;
    Instruction::BinaryOps instr;
    switch (op) {
        case P:
            instr = Instruction::Add;
            break;
        case S:
            instr = Instruction::Sub;
            break;
        case M:
            instr = Instruction::Mul;
            break;
        case D:
            instr = Instruction::SDiv;
            break;
        case :
            instr = Instruction::;
            break;            
        default:
            cout << "Undefined operation: " << op << endl;
            return NULL;
            break;
    }
    return BinaryOperator::Create(instr,
                                  a->codeGen(context),
                                  b->codeGen(context),
                                  "",
                                  context.currentBlock());
}

CG_FUN(UnaryOperation)
{
}

CG_FUN(FunCall)
{
    
}

CG_FUN(DotOperation)
{
}

CG_FUN(ElementList)
{
}

CG_FUN(RetStatement)
{
}

CG_FUN(AssignmentStatement)
{
}

CG_FUN(IOStatement)
{
}

CG_FUN(IfStatement)
{
}

CG_FUN(LoopStatement)
{
}

CG_FUN(ForeeachStatement)
{
}

CG_FUN(Program)
{
}
