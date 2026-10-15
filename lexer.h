#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum Token {
  tok_eof = -1,
  tok_def = -2,
  tok_extern = -3,
  tok_identifier = -4,
  tok_number = -5,
};

char *IdentifierStr;
double NumVal;

int gettok();
