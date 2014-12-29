#include <iostream>
#include <vector>

#include "ast.h"
#include "codegen.h"
#include "present.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::list;
CGBlock::CGBlock(BasicBlock *_block, Value *_value):
        block(_block), retValue(_value) {}

CGContext::CGContext():
        module(new Module("main", getGlobalContext())) {}

Function *printFn = nullptr;
Function *scanFn = nullptr;

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

void CGContext::pushBlock(BasicBlock *block, Local l)
{
    blocks.push(new CGBlock(block, nullptr));
    locals() = l;
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

bool CGContext::generateCode(ASTNode *root)
{
    cout << "Generating binary..." << endl;

    if (root->codeGen(*this) == nullptr) {
        cerr << "Something goes wrong!" << endl;
        return false;
    }

    cout << "Code generated." << endl << endl << endl;
    cout << "Module dump: " << endl << endl;
    module->dump();
    return true;
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
    cerr << "Err: Definition: Cannot be here! " << endl;
    return nullptr;
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
            Value *val = decl->codeGen(context);
            if (val == nullptr) {
                return false;
            }
            context.locals()[decl->name->name] = val;
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
    Value *last;
    // set arguments and put into locals
    if (args_var != nullptr) {
        Function::arg_iterator iter = function->arg_begin();
        for (std::list<ASTNode *>::iterator itr = args_var->elements.begin();
             itr != args_var->elements.end(); ++itr, ++iter) {
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
            string name = decl->name->name;
            cout << "arg: " << name << endl;
            iter->setName(name);

            if (!codeGen4VariableDeclarations(args_var, context)) {
                return nullptr;
            }
            last = new StoreInst(iter, context.locals()[name], false, context.currentBlock());
        }
    }
    // put the variable declarations into locals
    if (!codeGen4VariableDeclarations(variables, context)) {
        return nullptr;
    }
    // function body
    last = nullptr;
    if (functionBlock != nullptr) {
        last = functionBlock->codeGen(context);
        if (last == nullptr) {
            return nullptr;
        }
    }
    if (retType == nullptr) {
        RetStatement *ret = new RetStatement;
        NumericLiteral *zero = new NumericLiteral;
        zero->val = 0;
        ret->expr = zero;
        last = ret->codeGen(context);
        delete zero;
        delete ret;
        if (last == nullptr) {
            return nullptr;
        }
    }

    context.popBlock();
    return last;
}

CG_FUN(Statement)
{
    cerr << "Err: Statement: cannot be here!" << endl;
    return nullptr;
}

CG_FUN(Expression)
{
    cerr << "Err: Expression: cannot be here!" << endl;
    return nullptr;
}

CG_FUN(Primary)
{
    return expr->codeGen(context);
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
    Instruction::OtherOps other;
    CmpInst::Predicate pred;
    Value *va, *vb;
    switch (op) {
        case P:
            instr = Instruction::Add;
            goto math;
            break;
        case S:
            instr = Instruction::Sub;
            goto math;
            break;
        case M:
            instr = Instruction::Mul;
            goto math;
            break;
        case D:
            instr = Instruction::SDiv;
            goto math;
            break;
        case MOD:
            instr = Instruction::SRem;
            goto math;
            break;
        case E:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_EQ;
            goto relation;
            break;
        case NE:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_NE;
            goto relation;
            break;
        case L:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_SLT;
            goto relation;
            break;
        case LE:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_SLE;
            goto relation;
            break;
        case G:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_SGT;
            goto relation;
            break;
        case GE:
            other = Instruction::ICmp;
            pred = CmpInst::ICMP_SGE;
            goto relation;
            break;
        case A:
            instr = Instruction::And;
            goto math;
            break;            
        case O:
            instr = Instruction::Or;
            goto math;
            break;       
        case NONE:
            return a->codeGen(context);
            break;
        default:
            cerr << "Undefined operation: " << op << endl;
            return nullptr;
            break;
    }
math:
    va = a->codeGen(context);
    if (va == nullptr) {
        return nullptr;
    }
    vb = b->codeGen(context);
    if (vb == nullptr) {
        return nullptr;
    }
    return BinaryOperator::Create(instr,
                                  va,
                                  vb,
                                  "",
                                  context.currentBlock());
relation:
    va = a->codeGen(context);
    if (va == nullptr) {
        return nullptr;
    }
    vb = b->codeGen(context);
    if (vb == nullptr) {
        return nullptr;
    }
    CmpInst *result = CmpInst::Create(other, pred, va, vb, "cmptmp", context.currentBlock());
    return result;
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
    cout << "Generating function call..." << endl;
    // get function by string temporarily
    if (!IS_PANY(name, IdentPr)) {
        cerr << "Err: Not implemented function calls except from a string name" << endl;
        return nullptr;
    }
    string fname = ((IdentPr *)name)->name->name;
    Function *function = context.module->getFunction(fname.c_str());
    if (function == nullptr) {
        cerr << "Semantic error: no such function" << fname << endl;
        return nullptr;
    }
    vector<Value *> cargs;
    if (args != nullptr) {
        for (list<ASTNode *>::iterator itr = args->elements.begin();
             itr != args->elements.end(); ++itr) {
            Value *val = (*itr)->codeGen(context);
            if (val == nullptr) {
                return nullptr;
            }
            val->dump();
            cargs.push_back(val);
        }
    }
    return CallInst::Create(function, cargs, "", context.currentBlock());
}

CG_FUN(DotOperation)
{
}

CG_FUN(ElementList)
{
    Value *last;
    cout << "Generating list..." << endl;
    for (std::list<ASTNode *>::iterator itr = elements.begin();
         itr != elements.end(); ++itr) {
        last = (*itr)->codeGen(context);
        if (last == nullptr) {
            return nullptr;
        }
    }
    return last;
}

CG_FUN(RetStatement)
{
    cout << "Generating return code..." << endl;
    if (expr == nullptr) {
        return ReturnInst::Create(getGlobalContext(), context.currentBlock());
    } else {
        Value *ret = expr->codeGen(context);
        if (ret == nullptr) {
            return nullptr;
        }
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
    if (right == nullptr) {
        return nullptr;
    }
    
    return new StoreInst(right, left, false, context.currentBlock());
}

Function* createScanfFunction(CGContext& context)
{
    vector<Type *> scanf_arg_types;
    scanf_arg_types.push_back(Type::getInt8PtrTy(getGlobalContext()));
    FunctionType* scanf_type =
            FunctionType::get(Type::getInt32Ty(getGlobalContext()),
                              scanf_arg_types,
                              true);
    Function *func = Function::Create(scanf_type,
                                      Function::ExternalLinkage,
                                      Twine("scanf"),
                                      context.module);
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

Function* createPrintfFunction(CGContext& context)
{
    std::vector<llvm::Type*> printf_arg_types;
    printf_arg_types.push_back(llvm::Type::getInt8PtrTy(getGlobalContext())); //char*

    llvm::FunctionType* printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(getGlobalContext()), printf_arg_types, true);

    llvm::Function *func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("printf"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

CG_FUN(IOStatement)
{
    vector<Type *> argTypes;
    vector<string> argNames;
    string format;
    Value *val;
    Type *type;
    Constant *format_const;
    GlobalVariable *_var;
    Constant *zero;
    vector<Constant *> indices;
    Constant *var_ref;
    vector<Value *> args;
    Function *fn;
    CallInst *call;
    switch (op) {
        case IN:
            cout << "Generating input statement..." << endl;
            val = var->codeGenRef(context);
            type = val->getType();
            argTypes.push_back(type);
            format = "%lld\n";
            format_const = ConstantDataArray::getString(getGlobalContext(), format);
            _var = new GlobalVariable(*context.module,
                                     ArrayType::get(IntegerType::get(getGlobalContext(), 8),
                                                    format.length() + 1),
                                     true,
                                     GlobalValue::PrivateLinkage,
                                     format_const,
                                     ".str");
            zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
            indices.push_back(zero);
            indices.push_back(zero);
            var_ref = ConstantExpr::getGetElementPtr(_var, indices);
            args.push_back(var_ref);
            args.push_back(val);
            if (scanFn == nullptr) {
                scanFn = createScanfFunction(context);
            }
            fn = scanFn;
            call = CallInst::Create(fn, makeArrayRef(args), "", context.currentBlock());
            break;
            break;
        case OUT:
            cout << "Generating output statement..." << endl;
            val = content->codeGen(context);
            type = val->getType();
            argTypes.push_back(type);
            format = "%lld\n";
            format_const = ConstantDataArray::getString(getGlobalContext(), format);
            _var = new GlobalVariable(*context.module,
                                     ArrayType::get(IntegerType::get(getGlobalContext(), 8),
                                                    format.length() + 1),
                                     true,
                                     GlobalValue::PrivateLinkage,
                                     format_const,
                                     ".str");
            zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
            indices.push_back(zero);
            indices.push_back(zero);
            var_ref = ConstantExpr::getGetElementPtr(_var, indices);
            args.push_back(var_ref);
            args.push_back(val);
            if (printFn == nullptr) {
                printFn = createPrintfFunction(context);
            }
            fn = printFn;
            call = CallInst::Create(fn, makeArrayRef(args), "", context.currentBlock());
            break;
        default:
            cerr << "Err: unknown IO operator!" << endl;
            return nullptr;
            break;
    }
}

CG_FUN(IfStatement)
{
    cout << "Generating if statement.. " << endl;

    if (conds->elements.begin() == conds->elements.end()) {
        return nullptr;
    }
    Value *condValue;
    if (*(conds->elements.begin()) != nullptr) {
        condValue = (*(conds->elements.begin()))->codeGen(context);
    } else {
        condValue = ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
                                     1,
                                     true);
    }
    ElementList *block = *(stats.begin());
    conds->elements.pop_front();
    stats.pop_front();

    condValue = new FCmpInst(*context.currentBlock(),
                             CmpInst::FCMP_ONE,
                             condValue,
                             ConstantFP::get(getGlobalContext(),
                                             APFloat(0.0)));

    Local l = context.locals();
    Function *function = context.currentBlock()->getParent();

    BasicBlock *thenBlock = BasicBlock::Create(getGlobalContext(), "if.then", function);
    BasicBlock *elseBlock = BasicBlock::Create(getGlobalContext(), "if.else");    
    BasicBlock *mergeBlock = BasicBlock::Create(getGlobalContext(), "if.cont");
    BranchInst::Create(thenBlock, elseBlock, condValue, context.currentBlock());
    context.popBlock();
    context.pushBlock(thenBlock, l);
    Value *thenValue = block->codeGen(context);
    if (thenValue == nullptr) {
        return nullptr;
    }
    BranchInst::Create(mergeBlock, context.currentBlock());
    context.popBlock();

    function->getBasicBlockList().push_back(elseBlock);
    context.pushBlock(elseBlock, l);
    Value *elseValue = codeGen(context);
    if (elseValue != nullptr) {
        BranchInst::Create(mergeBlock, context.currentBlock());
    } else {
        function->getBasicBlockList().pop_back();
    }
    context.popBlock();

    function->getBasicBlockList().push_back(mergeBlock);
    context.pushBlock(mergeBlock, l);
    PHINode *PN = PHINode::Create(Type::getDoubleTy(getGlobalContext()),
                                  2,
                                  "if.tmp",
                                  context.currentBlock());
    PN->addIncoming(thenValue, thenBlock);
    if (elseValue != nullptr) {
        PN->addIncoming(elseValue, elseBlock);
    }
    return PN;
}

CG_FUN(LoopStatement)
{
    // cout << "Generating loop statement..." << endl;
    // Function *function = context.currentBlock()->getParent();
    // BasicBlock *condBlock = BasicBlock::Create(getGlobalContext(), "loop.cond", function);
    // BasicBlock *loopBlock = BasicBlock::Create(getGlobalContext(), "loop.loop");
    // BasicBlock *afterBlock = BasicBlock::Create(getGlobalContext(), "loop.after");

    // if (type == WHILE) {
    //     BranchInst::Create(condBlock, context.currentBlock());
    // } else {
    //     BranchInst::Create(loopBlock, context.currentBlock());
    // }
    
    // context.push(condBlock);
    // Value *condValue = cond->codeGen(context);
    // if (condValue == nullptr) {
    //     context.pop();
    //     function->getBasicBlockList().pop_back();
    //     return nullptr;
    // }
    // if (type == WHILE) {
    //     condValue = CmpInst::Create(Instruction::ICmp,
    //                                 CmpInst::ICMP_NE,
    //                                 condValue,
    //                                 ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
    //                                                  0,
    //                                                  true),
    //                                 "cmptmp",
    //                                 context.currentBlock());
    // } else {
    //     condValue = CmpInst::Create(Instruction::ICmp,
    //                                 CmpInst::ICMP_EQ,
    //                                 condValue,
    //                                 ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
    //                                                  0,
    //                                                  true),
    //                                 "cmptmp",
    //                                 context.currentBlock());
        
    // }
    // BranchInst::Create(loopBlock, afterBlock, condValue, context.currentBlock());
    // context.pop();

    // function->getBasicBlockList().push_back(loopBlock);
    // context.push(loopBlock);
    // Value *loopValue = stats->codeGen(context);
    // if (loopValue == nullptr) {
    //     context.pop();
    //     function->getBasicBlockList().pop_back();
    //     function->getBasicBlockList().pop_back();
    //     return nullptr;
    // }
    // BranchInst::Create(condBlock, context.currentBlock());
    // context.pop();

    // function->getBasicBlockList().push_back(afterBlock);
    // context.push(afterBlock);
    // PHINode *PN = PHINode::Create(Type::getVoidTy(getGlobalContext()), 2, "while.tmp", afterBlock);
    // PN->addIncoming(loopValue, loopBlock);
    // PN->addIncoming(condValue, condBlock);
    // //    ReturnInst::Create(getGlobalContext(), PN, afterBlock);

    // //    context.pop();
    // return PN;
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

    // function, class and array definitions
    if (definitions != nullptr && definitions->codeGen(context) == nullptr) {
        return nullptr;
    }

    // Variable definitions
    if (!codeGen4VariableDeclarations(variableDeclarations, context)) {
        cerr << "Failed to generate main function." << endl;
        return nullptr;
    }
    // main body of main function
    Value *last = nullptr;
    if (programBlock != nullptr) {
        last = programBlock->codeGen(context);
        if (last == nullptr) {
            return nullptr;
        }        
    }
    RetStatement *ret = new RetStatement;
    NumericLiteral *zero = new NumericLiteral;
    zero->val = 0;
    ret->expr = zero;
    last = ret->codeGen(context);
    if (last == nullptr) {
        return nullptr;
    }
    
    delete zero;
    delete ret;

    context.popBlock();
    return last;
}

CG_FUN(FuncStatement)
{
    return call->codeGen(context);
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