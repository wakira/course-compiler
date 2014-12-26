#include <iostream>
#include <vector>

#include "ast.h"
#include "codegen.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;

CGBlock::CGBlock(BasicBlock *_block, Value *_value):
        block(_block), retValue(_value) {}

CGContext::CGContext():
        module(new Module("main", getGlobalContext())) {}

Local &CGContext::locals()
{
    return blocks.top()->locals;
}

Types &CGContext::types()
{
    return blocks.top()->types;
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

MyType *CGContext::typeOf(string name)
{
    if (name.compare("integer") == 0) {
        MyType *ret = new MyType;
        ret->llvm_type = Type::getInt64Ty(getGlobalContext());
        ret->type = MyType::INT;
    } else if (types().find(name) != types().end()) {
        return types()[name];
    } else {
        return NULL;
    }
    
}

CG_FUN(VariableDeclaration)
{
    cout << "Generating variable declaration <" << type->name << ">"
         << name->name << endl;
    AllocaInst *alloc = new AllocaInst(context.typeOf(type->name)->llvm_type, name->name.c_str(),
                                       context.currentBlock());
    context.locals()[name->name] = alloc;
    return alloc;
}

CG_FUN(Definition)
{
}

CG_FUN(ArrayDefinition)
{
    cout << "Creating array definition for " << name->name << "<" << type->name << ">" << endl;
    if (context.typeOf(type->name) == NULL || size <= 0) {
        cerr << "Wrong definition of array: " << type->name << "[" << size << "]" << endl;
        return NULL;
    } else if (context.typeOf(name->name) != NULL) {
        cerr << "Multiple definition of " << name->name << endl;
        return NULL;
    } else {
        MyType *arr_ty = new MyType;
        arr_ty->type = MyType::ARRAY;
        arr_ty->llvm_type = ArrayType::get(context.typeOf(type->name)->llvm_type, size);
        context.types()[name->name] = arr_ty;
        return (Value *)arr_ty->llvm_type;
    }
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
    cerr << "Not expected!" << endl;
    return this->codeGen(context);
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
    return new LoadInst(context.locals()[name->name],
                        "",
                        false,
                        context.currentBlock());
}

CG_FUN(ArrayPr)
{
}

CG_FUN(NumericLiteral)
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
        case MOD:
            instr = Instruction::SRem;
            break;
        case A:
            instr = Instruction::And;
            break;            
        case O:
            instr = Instruction::Or;
            break;            
        // case :
        //     instr = Instruction::;
        //     break;                        
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
    cout << "Generating list..." << endl;
    for (std::list<ASTNode *>::iterator itr = elements.begin();
         itr != elements.end(); ++itr) {
        (*itr)->codeGen(context);
    }
}

CG_FUN(RetStatement)
{
    cout << "Generating return code..." << endl;
    Value *ret = expr->codeGen(context);
    context.setCurrentRetValue(ret);
    return ret;
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

CG_FUN(ForeachStatement)
{
}

CG_FUN(Program)
{
}

CG_FUN(FuncStatement)
{
}
