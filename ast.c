#include "ast.h"
#include "parser.h"
#include <llvm-c/Types.h>
#include <stdlib.h>
#include <llvm-c/Core.h>

struct ExprAST *newExprAST() {
  return malloc(sizeof(struct ExprAST));
}

struct PrototypeAST *newPrototypeAST(char *name, struct list *args) {
  struct PrototypeAST *Result = malloc(sizeof(struct PrototypeAST));
  Result->Name = name;
  Result->Args = args;
  return Result;
}

struct FunctionAST *newFunctionAST(struct PrototypeAST *proto, struct ExprAST *body) {
  struct FunctionAST *Result = malloc(sizeof(struct FunctionAST));
  Result->Proto = proto;
  Result->Body = body;
  return Result;
};

LLVMValueRef LogErrorV(char *Str) {
  LogError(Str);
  return NULL;
}

LLVMContextRef context = NULL; 
LLVMBuilderRef builder = NULL;
LLVMModuleRef module = NULL;
struct list *NamedValues = NULL;

LLVMValueRef Number_codegen(struct ExprAST *ast) {
  return LLVMConstReal(LLVMDoubleTypeInContext(context), ast->Val);
}

LLVMValueRef Variable_codegen(struct ExprAST *ast) {
  LLVMValueRef V = map_get(NamedValues, ast->Name);
  if (!V) {
    return LogErrorV("Unknown variable name");
  }
  return V;
}

LLVMValueRef Binary_codegen(struct ExprAST *ast) {
  LLVMValueRef L = codegen(ast->LHS);
  LLVMValueRef R = codegen(ast->RHS);
  if (!L || !R) {
    return NULL;
  }
  switch (ast->Op) {
  case '+':
    return LLVMBuildFAdd(builder, L, R, "addtmp");
  case '-':
    return LLVMBuildFSub(builder, L, R, "subtmp");
  case '*':
    return LLVMBuildFMul(builder, L, R, "multmp");
  case '<':
    L = LLVMBuildFCmp(builder, LLVMRealULT, L, R, "cmptmp");
    return LLVMBuildUIToFP(builder, L, LLVMDoubleTypeInContext(context), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}

LLVMValueRef Call_codegen(struct ExprAST * ast) {
  LLVMValueRef CalleeF = LLVMGetNamedFunction(module, ast->Callee);
  if (!CalleeF) {
    return LogErrorV("Unknown function referenced");
  }
  int args_size = size(ast->Args);
  if (LLVMCountParams(CalleeF) != args_size) {
    return LogErrorV("Incorrect number of arguments passed");
  }
  LLVMValueRef *ArgsV = malloc(sizeof(LLVMValueRef) * args_size);
  struct list *e = ast->Args;
  for (int i = 0; i < args_size; i++) {
    LLVMValueRef v = codegen(e->value);
    if (!v) {
      free(ArgsV);
      return NULL;
    }
    ArgsV[i] = v;
    e = e->next;
  }
  return LLVMBuildCall2(builder, LLVMGlobalGetValueType(CalleeF), CalleeF,
                        ArgsV, args_size, "calltmp");
}

LLVMValueRef codegen(struct ExprAST *ast) {
  if (context == NULL) {
    context = LLVMContextCreate();
    builder = LLVMCreateBuilderInContext(context);
    module = LLVMModuleCreateWithNameInContext("foo", context);
  }

  switch (ast->Type) {
  case Number:
    return Number_codegen(ast);
  case Variable:
    return Variable_codegen(ast);
  case Binary:
    return Binary_codegen(ast);
  case Call:
    return Call_codegen(ast);
  default:
    return NULL;
  }
}

void destroy() {
  if (context != NULL) {
    LLVMContextDispose(context);
    context = NULL;
    LLVMDisposeBuilder(builder);
    builder = NULL;
    LLVMDisposeModule(module);
    module = NULL;
  }
}
