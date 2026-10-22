#include "ast.h"

struct ExprAST *ParseExpression();
struct ExprAST *LogError(char *Str);
int getNextToken();
void Driver();
