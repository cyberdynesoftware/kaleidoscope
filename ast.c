#include "ast.h"
#include <stdlib.h>

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
