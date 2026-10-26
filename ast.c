#include "ast.h"
#include "parser.h"
#include <llvm-c/Types.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <string.h>
#include <stdio.h>

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

void init() {
  if (context == NULL) {
    context = LLVMContextCreate();
    builder = LLVMCreateBuilderInContext(context);
    module = LLVMModuleCreateWithNameInContext("foo", context);
  }
}

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
    // printf("Looking up %s failed!\n", ast->Callee);
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

LLVMValueRef Proto_codegen(struct PrototypeAST *ast) {
  int args_size = size(ast->Args);
  LLVMTypeRef *Doubles = malloc(sizeof(LLVMTypeRef) * args_size);
  for (int i = 0; i < args_size; i++) {
    Doubles[i] = LLVMDoubleTypeInContext(context);
  }
  LLVMTypeRef FT =
      LLVMFunctionType(LLVMDoubleTypeInContext(context), Doubles, args_size, 0);
  LLVMValueRef F = LLVMAddFunction(module, ast->Name, FT);

  if (LLVMCountParams(F) > 0) {
    LLVMValueRef param = LLVMGetFirstParam(F);
    struct list *e = ast->Args;
    LLVMSetValueName2(param, e->value, strlen(e->value));
    while (param != LLVMGetLastParam(F)) {
      e = e->next;
      param = LLVMGetNextParam(param);
      LLVMSetValueName2(param, e->value, strlen(e->value));
    }
  }
  return F;
}

char *param_name(LLVMValueRef param) {
  size_t param_name_length;
  const char *raw_param_name = LLVMGetValueName2(param, &param_name_length);
  char *param_name_copy = malloc(param_name_length + 1);
  strncpy(param_name_copy, raw_param_name, param_name_length);
  param_name_copy[param_name_length] = '\0';
  return param_name_copy;
}

LLVMValueRef Function_codegen(struct FunctionAST *ast) {
  LLVMValueRef TheFunction = LLVMGetNamedFunction(module, ast->Proto->Name);
  if (!TheFunction) {
    TheFunction = Proto_codegen(ast->Proto);
  }
  if (!TheFunction) {
    return NULL;
  }
  if (LLVMCountBasicBlocks(TheFunction) > 0) {
    return LogErrorV("Function cannot be redefined.");
  }
  LLVMBasicBlockRef BB = LLVMAppendBasicBlockInContext(context, TheFunction, "entry");
  LLVMPositionBuilderAtEnd(builder, BB);

  if (NamedValues != NULL) {
    free(NamedValues);
    NamedValues = NULL;
  }
  if (LLVMCountParams(TheFunction) > 0) {
    LLVMValueRef param = LLVMGetFirstParam(TheFunction);
    NamedValues = new_map_element(param_name(param), param);
    while (param != LLVMGetLastParam(TheFunction)) {
      param = LLVMGetNextParam(param);
      map_insert(NamedValues, param_name(param), param);
    }
  }
  LLVMValueRef RetVal = codegen(ast->Body);
  if (RetVal) {
    LLVMBuildRet(builder, RetVal);
    return TheFunction;
  }
  LLVMDeleteFunction(TheFunction);
  return NULL;
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
