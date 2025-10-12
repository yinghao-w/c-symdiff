#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.h"
#include "symbols.h"
#include "transforms.h"
#include <stdio.h>

void wrapper(Ast_Node *node, void *ctx) {
	(void)ctx;
	token_print(node->value);
	printf(" ");
}

int main(int argc, char *argv[]) {
  opr_set_init();
  simpls_init();
  rules_init();
  diff_rules_init();

  Expression expr = expr_create(argv[1]);

  printf("Input expression: ");
  ast_iter_apply(get_root(expr), T_POST, wrapper, NULL);
  printf("\n");

  printf("Normalised expression: ");
  norm_apply(expr);
  ast_iter_apply(get_root(expr), T_POST, wrapper, NULL);
  printf("\n");

  printf("Differentiated expression: ");
  diff_apply(expr);
  ast_iter_apply(get_root(expr), T_POST, wrapper, NULL);
  printf("\n");
  return 0;
}
