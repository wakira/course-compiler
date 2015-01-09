#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "ast.h"
#include "codegen.h"
#include "present.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::list;

static void semanticError(string info) {
	cerr << "Semantic Error: " << info << endl;
	std::exit(1);
}

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

CGBlock CGContext::pop() {
  CGBlock top = *(blocks.top());
  popBlock();
  return top;
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
	assert(0); // Impossible
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
	assert(0); // Impossible
}

CG_FUN(ArrayDefinition)
{
    cout << "Creating array definition for " << name->name << "<" << type->name << ">" << endl;
    if (context.typeOf(type->name) == nullptr || size <= 0) {
		semanticError("Wrong definition of array " + name->name);
    } else if (context.typeOf(name->name) != nullptr) {
		semanticError("Multiple definition of " + name->name);
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
	assert(0); // Impossible
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
    if (args_var != nullptr) {
		if (arguments == nullptr || arguments->elements.size() != args_var->elements.size()) {
			semanticError("Function arguments doesn't match its arguments type declaration");
		}
        for (std::list<ASTNode *>::iterator itr = args_var->elements.begin(), itr_ck = arguments->elements.begin();
             itr != args_var->elements.end(); ++itr, ++itr_ck) {
			IdentPr *arg_check = (IdentPr *)*itr_ck;
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
			// Check if there's local variable declaration with same name as arguments
			for (auto loc_val : variables->elements) {
				IdentPr *loc = (IdentPr*)loc_val;
				if (loc->name->name == decl->name->name) {
					semanticError("Function local variable and arguments conflicts");
				}
			}
			// Check if argument list and their type declaration match
			if (decl->name->name != arg_check->name->name) {
				semanticError("Function arguments doesn't match its arguments type declaration");
			}
            MyType *t = context.typeOf(decl->type->name);
            Type *type = t->llvm_type;
            argTypes.push_back(type);
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
    Local l = context.locals();
    Types t = context.types();
    context.pushBlock(bblock);
    context.types() = t;
    Value *last;
    // set arguments and put into locals
    if (args_var != nullptr) {
        if (!codeGen4VariableDeclarations(args_var, context)) {
            return nullptr;
        }        
        Function::arg_iterator iter = function->arg_begin();
        for (std::list<ASTNode *>::iterator itr = args_var->elements.begin();
             itr != args_var->elements.end(); ++itr, ++iter) {
            VariableDeclaration *decl = (VariableDeclaration *)*itr;
            string name = decl->name->name;
            cout << "arg: " << name << endl;
            iter->setName(name);

            last = new StoreInst(iter, context.locals()[name], false, context.currentBlock());
            if (last == nullptr) {
				semanticError("Cannot load arguments!");
            }
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
	  //return nullptr;
        }
    }

    if (retType == nullptr) {
        RetStatement *ret = new RetStatement;
        ret->expr = nullptr;
        last = ret->codeGen(context);
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
	assert(0); // Impossible
}

CG_FUN(Expression)
{
	assert(0); // Impossible
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
		semanticError("Undeclared variable " + ident);
    }
    return new LoadInst(context.locals()[name->name],
                        "",
                        false,
                        context.currentBlock());
}

CG_FUN(ArrayPr)
{
    cout << "Generating array primary..." << endl;
    if (!IS_PANY(name, IdentPr)) {
		semanticError("Not implemented function calls except from a string name");
    }
    string fname = ((IdentPr *)name)->name->name;
    Value *i = index->codeGen(context);
    if (i == nullptr) {
		semanticError("Invalid array index");
    }
    if (context.locals().find(fname) == context.locals().end()) {
		semanticError("Array not defined");
    }
    vector<Value *> idxlist;
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
    idxlist.push_back(zero);
    idxlist.push_back(i);
    Value * ptr = context.locals()[fname];
    ptr = GetElementPtrInst::CreateInBounds(ptr,
                                            makeArrayRef(idxlist), "gep.tmp", context.currentBlock());

    ptr = new LoadInst(ptr,
                       "",
                       false,
                       context.currentBlock());
    return ptr;
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
            cerr << "Fatal Error: Undefined operation " << op << endl;
			assert(0); // Impossible operation
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
            cerr << "Fatal Error: Undefined operation " << op << endl;
			assert(0); // Impossible operation
            break;
    }
    
    return ret;
}

CG_FUN(FunCall)
{
    cout << "Generating function call..." << endl;
    // get function by string temporarily
    if (!IS_PANY(name, IdentPr)) {
		semanticError("Not implemented function calls except from a string name");
    }
    string fname = ((IdentPr *)name)->name->name;
    Function *function = context.module->getFunction(fname.c_str());
    if (function == nullptr) {
		semanticError("No such function named " + fname);
    }
	auto func_type = function->getFunctionType();
	if (func_type->getNumParams() != args->elements.size()) {
		semanticError("Function call with incorrect number of arguments");
	}
    vector<Value *> cargs;
    if (args != nullptr) {
		auto it = func_type->param_begin();
        for (list<ASTNode *>::iterator itr = args->elements.begin();
             itr != args->elements.end(); ++itr, ++it) {
            Value *val;
            val = (*itr)->codeGen(context);
			auto actual_arg_type_id = val->getType()->getTypeID();
			auto expected_type_id = (*it)->getTypeID();
			if (actual_arg_type_id != expected_type_id) {
				semanticError("Function call with incorrect argument type");
			}
            if (val == nullptr) {
                return nullptr;
            }
            //val->dump();
            cargs.push_back(val);
        }
    }
    return CallInst::Create(function, cargs, "", context.currentBlock());
}

CG_FUN(DotOperation)
{
	assert(0); // Not implemented yet
}

CG_FUN(ElementList)
{
    Value *last;
    cout << "Generating list..." << endl;
    for (std::list<ASTNode *>::iterator itr = elements.begin();
         itr != elements.end(); ++itr) {
        last = (*itr)->codeGen(context);
        if (last == nullptr) {
            //            return nullptr;
        }
    }
    return last;
}

CG_FUN(RetStatement)
{
    cout << "Generating return code..." << endl;
	Function *func = context.currentBlock()->getParent();
	auto expected_type_id = func->getReturnType()->getTypeID();
    if (expr == nullptr) {
		if (expected_type_id != Type::VoidTyID) {
			semanticError("Return statement requires expression on non-void function");
		}
        return ReturnInst::Create(getGlobalContext(), context.currentBlock());
    } else {
        Value *ret = expr->codeGen(context);
        if (ret == nullptr) {
			semanticError("Failed to generate return expression");
        }
		if (ret->getType()->getTypeID() != expected_type_id) {
			semanticError("Return expression has invalid type");
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
		semanticError("Undeclared variable");
    }
    if (rhs == nullptr) {
		semanticError("Non-existed right value");
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
            format = "%lld";
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
			assert(0); // Impossible!
            break;
    }
    return call;
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

    condValue = CmpInst::Create(Instruction::ICmp,
                                CmpInst::ICMP_NE,
                                condValue,
                                ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
                                                 0,
                                                 true),
                                "cmptmp",
                                context.currentBlock());

    Local l = context.locals();
    Function *function = context.currentBlock()->getParent();

    BasicBlock *thenBlock = BasicBlock::Create(getGlobalContext(), "if.then", function);
    BasicBlock *elseBlock = BasicBlock::Create(getGlobalContext(), "if.else");    
    BasicBlock *mergeBlock = BasicBlock::Create(getGlobalContext(), "if.cont");
    BranchInst::Create(thenBlock, elseBlock, condValue, context.currentBlock());

    CGBlock top = context.pop();
    context.pushBlock(thenBlock, l);
    Value *thenValue = block->codeGen(context);
    if (thenValue == nullptr) {
      //return nullptr;
    }
    BranchInst::Create(mergeBlock, context.currentBlock());
    context.popBlock();

    function->getBasicBlockList().push_back(elseBlock);
    context.pushBlock(elseBlock, l);
    Value *elseValue = codeGen(context);
    BranchInst::Create(mergeBlock, context.currentBlock());
    context.popBlock();

    function->getBasicBlockList().push_back(mergeBlock);
    context.pushBlock(mergeBlock, l);
    //    context.popBlock();
    // PHINode *PN = PHINode::Create(Type::getInt64Ty(getGlobalContext()),
    //                               2,
    //                               "if.tmp",
    //                               context.currentBlock());
    // PN->addIncoming(thenValue, thenBlock);
    // if (elseValue != nullptr) {
    //     PN->addIncoming(elseValue, elseBlock);
    // }
    return nullptr;
}

CG_FUN(LoopStatement)
{
    cout << "Generating loop statement..." << endl;
    Local l = context.locals();
    Function *function = context.currentBlock()->getParent();
    BasicBlock *condBlock = BasicBlock::Create(getGlobalContext(), "loop.cond", function);
    BasicBlock *loopBlock = BasicBlock::Create(getGlobalContext(), "loop.loop");
    BasicBlock *afterBlock = BasicBlock::Create(getGlobalContext(), "loop.after");
    if (type == WHILE) {
        BranchInst::Create(condBlock, context.currentBlock());
    } else {
        BranchInst::Create(loopBlock, context.currentBlock());
    }
    context.popBlock();

    context.pushBlock(condBlock, l);
    Value *condValue = cond->codeGen(context);
    if (condValue == nullptr) {
        context.popBlock();
        function->getBasicBlockList().pop_back();
        return nullptr;
    }
    if (type == WHILE) {
        condValue = CmpInst::Create(Instruction::ICmp,
                                    CmpInst::ICMP_NE,
                                    condValue,
                                    ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
                                                     0,
                                                     true),
                                    "cmptmp",
                                    context.currentBlock());
    } else {
        condValue = CmpInst::Create(Instruction::ICmp,
                                    CmpInst::ICMP_EQ,
                                    condValue,
                                    ConstantInt::get(Type::getInt64Ty(getGlobalContext()),
                                                     0,
                                                     true),
                                    "cmptmp",
                                    context.currentBlock());
        
    }
    BranchInst::Create(loopBlock, afterBlock, condValue, context.currentBlock());
    context.popBlock();

    function->getBasicBlockList().push_back(loopBlock);
    context.pushBlock(loopBlock, l);
    Value *loopValue = stats->codeGen(context);
    if (loopValue == nullptr) {
        // context.popBlock();
        // function->getBasicBlockList().pop_back();
        // function->getBasicBlockList().pop_back();
        // return nullptr;
    }
    BranchInst::Create(condBlock, context.currentBlock());
    context.popBlock();

    function->getBasicBlockList().push_back(afterBlock);
    context.pushBlock(afterBlock, l);
    // PHINode *PN = PHINode::Create(Type::getVoidTy(getGlobalContext()), 2, "while.tmp", afterBlock);
    // PN->addIncoming(loopValue, loopBlock);
    // PN->addIncoming(condValue, condBlock);
    //    ReturnInst::Create(getGlobalContext(), PN, afterBlock);

    //    context.pop();
    return nullptr;
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

    ElementList *funclist = new ElementList;
    ElementList *typelist = new ElementList;
	if (definitions != nullptr) {
		for (list<ASTNode *>::iterator itr = definitions->elements.begin();
			 itr != definitions->elements.end(); ++itr) {
			if (IS_SAME_TYPE(**itr, *(new FunctionDefinition()))) {
				cout << "funtion!" << endl;
				funclist->elements.push_back(*itr);
			} else {
				typelist->elements.push_back(*itr);
			}
		}
	}

    // class and array definitions
    if (typelist != nullptr && typelist->codeGen(context) == nullptr) {
    }

    // Variable definitions
    if (!codeGen4VariableDeclarations(variableDeclarations, context)) {
        cerr << "Failed to generate main function." << endl;
        return nullptr;
    }
    
    if (funclist != nullptr && funclist->codeGen(context) == nullptr) {

    }
    
    // main body of main function
    Value *last = nullptr;
    if (programBlock != nullptr) {
        last = programBlock->codeGen(context);
        if (last == nullptr) {
            // return nullptr;
        }        
    }
    RetStatement *ret = new RetStatement;
    ret->expr = nullptr;
    last = ret->codeGen(context);
    
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
	semanticError("Cannot generate reference from Primary");
}
CGR_FUN(ArrayPr)
{
    cout << "Generating array primary ref..." << endl;
    if (!IS_PANY(name, IdentPr)) {
		semanticError("Not implemented function calls except from a string name");
    }
    string fname = ((IdentPr *)name)->name->name;
    Value *i = index->codeGen(context);
    if (i == nullptr) {
        return nullptr;
    }
    if (context.locals().find(fname) == context.locals().end()) {
		semanticError("Array not defined");
    }
    vector<Value *> idxlist;
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
    idxlist.push_back(zero);
    idxlist.push_back(i);
    Value * ptr = context.locals()[fname];
    ptr = GetElementPtrInst::CreateInBounds(ptr,
                                            makeArrayRef(idxlist), "gep.tmp", context.currentBlock());

    return ptr;
}
CGR_FUN(IdentPr)
{
    string ident = name->name;
    cout << "Generating identifier: " << ident << endl;
    if (context.locals().find(ident) == context.locals().end()) {
		semanticError("Undeclared variable " + ident);
    }
    return context.locals()[name->name];
}
