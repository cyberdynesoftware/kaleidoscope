#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LastChar = ' ';
char *IdentifierStr;
double NumVal;

char *getIdentifierStr() {
  return IdentifierStr;
}

double getNumVal() {
  return NumVal;
}

int gettok() {
  // printf("=gettok=\n");

  while (isspace(LastChar)) {
    LastChar = getchar();
  }
  
  if (isalpha(LastChar)) {
    IdentifierStr = malloc(512);
    IdentifierStr[0] = LastChar;

    for (int i = 1;; i++) {
      LastChar = getchar();
      if (isalnum(LastChar)) {
        IdentifierStr[i] = LastChar;
      } else {
        IdentifierStr[i] = '\0';
        IdentifierStr = realloc(IdentifierStr, i + 1);
        break;
      }
    }

    if (strcmp(IdentifierStr, "def") == 0) {
      free(IdentifierStr);
      // printf("=TOKEN= def\n");
      return tok_def;
    }
    if (strcmp(IdentifierStr, "extern") == 0) {
      free(IdentifierStr);
      // printf("=TOKEN= extern\n");
      return tok_extern;
    }

    // printf("=TOKEN= %s\n", getIdentifierStr());
    // printf("=LastChar= i %i - '%c'\n", LastChar, LastChar);
    return tok_identifier;
  }

  if (isdigit(LastChar)) {
    IdentifierStr = malloc(512);
    IdentifierStr[0] = LastChar;

    for (int i = 1;; i++) {
      LastChar = getchar();
      if (isdigit(LastChar) || LastChar == '.') {
        IdentifierStr[i] = LastChar;
      } else {
        IdentifierStr[i] = '\0';
        break;
      }
    }

    NumVal = strtod(IdentifierStr, 0);
    free(IdentifierStr);
    // printf("=TOKEN= %f\n", NumVal);
    return tok_number;
  }

  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF) {
      return gettok();
    }
  }

  if (LastChar == EOF) {
    // printf("=TOKEN= EOF\n");
    return tok_eof;
  }

  int result = LastChar;
  LastChar = getchar();
  // printf("=TOKEN= %c\n", result);
  // printf("=LastChar= i %i - '%c'\n", LastChar, LastChar);
  return result;
}
