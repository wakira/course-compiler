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
    blocks.push(new CGBlock(block, nullptr));
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

    if (root->codeGen(*this) == nullptr) {
        cerr << "Something goes wrong!" << endl;
        return;
    }

    cout << "Code generated." << endl << endl << endl;
    cout << "Module dump: " << endl << endl;
    module->dump();

}

GenericValue CGContext::runCode()
{
    cout << endl << "running..." << endl << endl;
    ExecutionEngine *ee = EngineBuilder(module).create();
    vector<GenericValue> noargs;
    GenericValue v = ee->runFunction(mainFunction, noargs);
    cout << endl << "done" << endl;
    return v;
}

#define CG_FUN(nodeType) Value *nodeType::codeGen(CGContext& context)
#define CGR_FUN(nodeType) Value *nodeType::codeGenRef(CGContext& context)

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
        return nullptr;
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
    if (context.typeOf(type->name) == nullptr || size <= 0) {
        cerr << "Wrong definition of array: " << type->name << "[" << size << "]" << endl;
        return nullptr;
    } else if (context.typeOf(name->name) != nullptr) {
        cerr << "Multiple definition of " << name->name << endl;
        return nullptr;
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

static bool codeGen4VariableDeclarations(ElementList *varlist, CGContext &context)
{
    if (varlist != nullptr) {
        for (std::list<ASTNode *>::iterator itr = varlist->elements.begin();
             itr != varlist->elements.end(); ++itr) {
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
            if (context.locals().find(decl->name->name) != context.locals().end()) {
                return false;
            }
            context.locals()[decl->name->name] = decl->codeGen(context);
        }
    }
    return true;
}

CG_FUN(FunctionDefinition)
{
    vector<Type *> argTypes;
    /*
      ToDo: Verify the correspondence of arguments and args_var.
     */
    ;
    if (args_var != nullptr) {
        for (std::list<ASTNode *>::iterator itr = args_var->elements.begin();
             itr != args_var->elements.end(); ++itr) {
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
            argTypes.push_back(context.typeOf(decl->type->name)->llvm_type);
        }
    }
    Type *retT = retType ==
            nullptr
            ? Type::getVoidTy(getGlobalContext())
            : context.typeOf(retType->name)->llvm_type;
    FunctionType *ftype = FunctionType::get(retT,
                                            argTypes,
                                            false);
    Function *function = Function::Create(ftype,
                                          GlobalValue::InternalLinkage,
                                          name->name.c_str(),
                                          context.module);

    if (function->getName() != name->name) {
        function->eraseFromParent();
        function = context.module->getFunction(name->name);
    }
    // create block
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(),
                                            "entry",
                                            function,
                                            0);
    context.pushBlock(bblock);

    // set arguments and put into locals
    if (args_var != nullptr) {
        Function::arg_iterator iter = function->arg_begin();
        for (std::list<ASTNode *>::iterator itr = args_var->elements.begin();
             itr != args_var->elements.end(); ++itr, ++iter) {
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
            string name = decl->name->name;
            cout << "arg: " << name << endl;
            iter->setName(name);

            context.locals()[name] = iter;
        }
    }
    // put the variable declarations into locals
    if (!codeGen4VariableDeclarations(variables, context)) {
        return nullptr;
    }
    // function body
    Value *last = nullptr;
    if (functionBlock != nullptr) {
        last = functionBlock->codeGen(context);
    }
    if (retType == nullptr) {
        RetStatement *ret = new RetStatement;
        NumericLiteral *zero = new NumericLiteral;
        zero->val = 0;
        ret->expr = zero;
        last = ret->codeGen(context);
        delete zero;
        delete ret;
    }

    context.popBlock();
    return last;
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
        return nullptr;
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
        case NONE:
            return a->codeGen(context);
            break;
        default:
            cout << "Undefined operation: " << op << endl;
            return nullptr;
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
    cout << "Generating unary operation: " << op << endl;
    BinaryOperation *bo;
    NumericLiteral *a;
    Value *ret;
    switch (op) {
        case BinaryOperation::S:
            bo = new BinaryOperation;
            a = new NumericLiteral;
            bo->b = p;
            bo->op = op;
            a->val = 0;
            bo->a = a;
            ret = bo->codeGen(context);
            delete a;
            delete bo;
            break;
        case BinaryOperation::NONE:
            ret = p->codeGen(context);
            break;
        default:
            cout << "Undefined operation: " << op << endl;
            delete a;
            return nullptr;
            break;
    }
    
    return ret;
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
    if (expr == nullptr) {
        return ReturnInst::Create(getGlobalContext(), context.currentBlock());
    } else {
        Value *ret = expr->codeGen(context);
        context.setCurrentRetValue(ret);
        return ReturnInst::Create(getGlobalContext(), ret, context.currentBlock());
    }
}

CG_FUN(AssignmentStatement)
{
    cout << "Generating assignment..." << endl;
    Value *left = lhs->codeGenRef(context);
    if (left == nullptr) {
        cerr << "Semantic error: undeclared variable" << endl;
        return nullptr;
    }
    if (rhs == nullptr) {
        cerr << "Semantic error: non-existed right value" << endl;
        return nullptr;
    }
    Value *right = rhs->codeGen(context);
    return new StoreInst(right, left, false, context.currentBlock());
}

CG_FUN(IOStatement)
{
}

CG_FUN(IfStatement)
{
    cout << "Generating if statement.. " << endl;
    cerr << "Do nothing now..." << endl;
    return nullptr;
}

CG_FUN(LoopStatement)
{
    cout << "Generating loop statement..." << endl;
    
}

CG_FUN(ForeachStatement)
{
}

CG_FUN(Program)
{
    cout << "Generating program: " << name->name << endl;
    // no args
    vector<Type *> argTypes;
    // no ret type
    Type *retType = Type::getVoidTy(getGlobalContext());
    FunctionType *ftype = FunctionType::get(retType, argTypes, false);
    Function *function = Function::Create(ftype,
                                          GlobalValue::InternalLinkage,
                                          name->name.c_str(),
                                          context.module);
    // set as main function
    context.mainFunction = function;
    // push a new block
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
    context.pushBlock(bblock);

    // Variable definitions
    if (!codeGen4VariableDeclarations(variableDeclarations, context)) {
        cerr << "Failed to generate main function." << endl;
        return nullptr;
    }
    // main body of main function
    Value *last = nullptr;
    if (programBlock != nullptr) {
        last = programBlock->codeGen(context);
    }
    RetStatement *ret = new RetStatement;
    NumericLiteral *zero = new NumericLiteral;
    zero->val = 0;
    ret->expr = zero;
    last = ret->codeGen(context);
    delete zero;
    delete ret;

    context.popBlock();
    return last;
}

CG_FUN(FuncStatement)
{
}

// CGR means generate the reference of a primary, rather load the value
CGR_FUN(Primary)
{
    cerr << "Err: cannot be possible to generate reference from Primary" << endl;
    return nullptr;
}
CGR_FUN(ArrayPr)
{
}
CGR_FUN(IdentPr)
{
    string ident = name->name;
    cout << "Generating identifier: " << ident << endl;
    if (context.locals().find(ident) == context.locals().end()) {
        cerr << "Err: Undeclared variable: " << ident << endl;
        return nullptr;
    }
    return context.locals()[name->name];
}
