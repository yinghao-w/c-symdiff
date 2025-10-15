#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.c"
#include <stdio.h>

int main(int argc, char *argv[]) {
  opr_set_init();
  simpls_init();
  norm_rules_init();
  diff_rules_init();

  while (1) {
    char input[100];
    fgets(input, 100, stdin);
    if (input[0] == 'q' && input[1] == '\n') {
      break;
    }
    Expression expr = expr_create(input);
    expr_print(expr);
    printf("\n");
    norm_apply(expr);
    expr_print(expr);
    printf("\n");
    diff_apply(expr);
    expr_print(expr);
    printf("\n");
  }

  return 0;
}
