#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include <ctype.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <string.h>

int CurTok;

int getNextToken() {
  return CurTok = gettok();
}

struct ExprAST *LogError(char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return NULL;
}

struct PrototypeAST *LogErrorP(char *Str) {
  LogError(Str);
  return NULL;
}

struct ExprAST *ParseNumberExpr() {
  struct ExprAST *Result = newExprAST();
  Result->Type = Number;
  Result->Val = getNumVal();
  getNextToken();
  return Result;
}

struct ExprAST *ParseParenExpr() {
  getNextToken();
  struct ExprAST *V = ParseExpression();

  if (!V) return NULL;
  if (CurTok != ')') return LogError("expected ')'");

  getNextToken();
  return V;  
}

struct ExprAST *ParseIdentifierExpr() {
  char *IdName = malloc(strlen(getIdentifierStr()) + 1);
  strcpy(IdName, getIdentifierStr());

  getNextToken();
  
  if (CurTok != '(') {
    struct ExprAST *Result = newExprAST();
    Result->Type = Variable;
    Result->Name = IdName;
    return Result;
  }

  getNextToken();

  struct list *Args = NULL;
  if (CurTok != ')') {
    while (1) {
      struct ExprAST *Arg = ParseExpression();
      if (Arg) {
        if (Args) {
          list_append_value(Args, Arg);
        } else {
          Args = new_list_element(Arg);
        }
      } else {
        return NULL;
      }
      if (CurTok == ')') break;
      if (CurTok != ',')
        return LogError(
            "ParseIdentifierExpr: Expected ')' or ',' in argument list");
      getNextToken();
    }
  }
  getNextToken();

  struct ExprAST *Result = newExprAST();
  Result->Type = Call;
  Result->Callee = IdName;
  Result->Args = Args;
  return Result;
}

struct ExprAST *ParsePrimary() {
  switch (CurTok) {
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  default:
    return LogError("unknown token when expecting an expression");
  }
}

int GetTopPrecedence() {
  if (!isascii(CurTok))
    return -1;

  switch (CurTok) {
  case '<':
    return 10;
  case '+':
  case '-':
    return 20;
  case '*':
    return 40;
  default:
    return -1;
  }
}

struct ExprAST *ParseBinOpRHS(int ExprPrec, struct ExprAST *LHS) {
  while (1) {
    int TokPrec = GetTopPrecedence();
    if (TokPrec < ExprPrec) {
      return LHS;
    }
    int BinOp = CurTok;
    getNextToken();

    struct ExprAST *RHS = ParsePrimary();
    if (!RHS) {
      return NULL;
    }
    int NextPrec = GetTopPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, RHS);
      if (!RHS) {
        return NULL;
      }
    }
    struct ExprAST *temp = newExprAST();
    temp->Type = Binary;
    temp->Op = BinOp;
    temp->LHS = LHS;
    temp->RHS = RHS;
    LHS = temp;
  }
}

struct ExprAST *ParseExpression() {
  struct ExprAST *LHS = ParsePrimary();
  if (!LHS) return NULL;
  return ParseBinOpRHS(0, LHS);
}

struct PrototypeAST *ParsePrototype() {
  // printf("=proto=\n");
  if (CurTok != tok_identifier) {
    return LogErrorP("Expected function name in prototype");
  }
  char *FnName = malloc(strlen(getIdentifierStr()) + 1);
  strcpy(FnName, getIdentifierStr());
  getNextToken();

  if (CurTok != '(') {
    return LogErrorP("Expected '(' in prototype");
  }
  struct list *ArgNames = NULL;
  while (getNextToken() == tok_identifier) {
    if (ArgNames == NULL) {
      ArgNames = new_list_element(getIdentifierStr());
    } else {
      list_append_value(ArgNames, getIdentifierStr());
    }
  }
  if (CurTok != ')') {
    return LogErrorP("Expected ')' in prototype");
  }
  getNextToken();
  return newPrototypeAST(FnName, ArgNames);
}

struct FunctionAST *ParseDefinition() {
  getNextToken();
  struct PrototypeAST *Proto = ParsePrototype();
  if (!Proto) {
    return NULL;
  }
  struct ExprAST *E = ParseExpression();
  if (!E) {
    return NULL;
  }
  return newFunctionAST(Proto, E);
}

struct PrototypeAST *ParseExtern() {
  getNextToken();
  return ParsePrototype();
}

struct FunctionAST *ParseTopLevelExpr() {
  struct ExprAST *E = ParseExpression();
  if (!E) {
    return NULL;
  }
  struct PrototypeAST *Proto = newPrototypeAST("__anon_expr", NULL);
  return newFunctionAST(Proto, E);
}

void HandleDefinition() {
  struct FunctionAST *FnAST = ParseDefinition();
  if (FnAST) {
    fprintf(stdout, "Parsed a function definition.\n");
    LLVMValueRef FnIR = Function_codegen(FnAST);
    if (FnIR) {
      printf("%s\n", LLVMPrintValueToString(FnIR));
    }
  } else {
    getNextToken();
  }
}

void HandleExtern() {
  struct PrototypeAST *ProtoAST = ParseExtern();
  if (ProtoAST) {
    fprintf(stdout, "Parsed an extern.\n");
    LLVMValueRef FnIR = Proto_codegen(ProtoAST);
    if (FnIR) {
      printf("%s\n", LLVMPrintValueToString(FnIR));
    }
  } else {
    getNextToken();
  }
}

void HandleTopLevelExpression() {
  struct FunctionAST *FnAST = ParseTopLevelExpr();
  if (FnAST) {
    fprintf(stdout, "Parsed a top-level expr.\n");
    LLVMValueRef FnIR = Function_codegen(FnAST);
    if (FnIR) {
      printf("%s\n", LLVMPrintValueToString(FnIR));
      LLVMDeleteFunction(FnIR);
    }
  } else {
    getNextToken();
  }
}

void Driver() {
  init();
  while (1) {
    fprintf(stdout, "kal> ");
    switch (CurTok) {
    case tok_eof:
      return;
    case ';':
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
  destroy();
}
