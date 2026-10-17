#include "parser.h"
#include <stdio.h>

int main() {
  fprintf(stdout, "kal> ");
  getNextToken();
  Driver();
  return 0;
}
