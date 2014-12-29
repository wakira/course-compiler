#ifndef _CODEGEN_HEADER_
#define _CODEGEN_HEADER_

#include <stack>
#include <map>
#include <string>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

class ASTNode;

typedef std::map<std::string, Value *> Local;

class MyType {
  public:
    enum { INT, ARRAY, CLASS } type;
    Type *llvm_type;
};

typedef std::map<std::string, MyType *> Types;

class CGBlock {
  public:
    CGBlock(BasicBlock *_block, Value *_value);
    BasicBlock *block;
    Value *retValue;
    Local locals;
    Types types;
};

class CGContext {
  private:
    std::stack<CGBlock *> blocks;
  public:
    Function *mainFunction;
    Module *module;
    CGContext();
    bool generateCode(ASTNode *root);
    GenericValue runCode();
    Local &locals();
    Types &types();
    BasicBlock *currentBlock();
    void pushBlock(BasicBlock *block);
    void push(BasicBlock *block);
    void popBlock();
    void pop();
    MyType *typeOf(std::string);
    void setCurrentRetValue(Value *value);
    Value *getCurrentRetValue();
};

#endif
