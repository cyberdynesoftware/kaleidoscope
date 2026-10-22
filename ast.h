#ifndef _AST_H_
#define _AST_H_

#include "list.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

typedef enum ExprASTType { Number, Variable, Binary, Call } exprAST_t;

struct ExprAST {
  exprAST_t Type;
  
  double Val;
  char *Name;

  char Op;
  struct ExprAST *LHS;
  struct ExprAST *RHS;

  char *Callee;
  struct list *Args;
};

struct PrototypeAST {
  char *Name;
  struct list *Args;
};

struct FunctionAST {
  struct PrototypeAST *Proto;
  struct ExprAST *Body;
};

struct ExprAST *newExprAST();
struct PrototypeAST *newPrototypeAST(char *name, struct list *args);
struct FunctionAST *newFunctionAST(struct PrototypeAST *proto, struct ExprAST *body);

LLVMValueRef codegen(struct ExprAST *ast);
void destroy(void);

#endif
