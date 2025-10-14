#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.h"
#include "symbols.h"
#include "transforms.h"
#include <stdio.h>

void recursive_print(Ast_Node *node) {
  if (ast_is_leaf(node)) {
    printf(" ");
    tok_print(node->value);
    printf(" ");
    return;
  }

  printf("(");
  if (node->lchild) {
    recursive_print(node->lchild);
  }
  printf(" ");
  tok_print(node->value);
  printf(" ");
  if (node->rchild) {
    recursive_print(node->rchild);
  }
  printf(")");
}

int main(int argc, char *argv[]) {
  opr_set_init();
  simpls_init();
  rules_init();
  diff_rules_init();

  while (1) {
    char input[100];
	fgets(input, 100, stdin);
	if (input[0] == 'q' && input[1] == '\n') {
		break;
	}
    Expression expr = expr_create(input);
    recursive_print(get_root(expr));
	printf("\n");
    norm_apply(expr);
    recursive_print(get_root(expr));
	printf("\n");
    diff_apply(expr);
    recursive_print(get_root(expr));
	printf("\n");
  }

  return 0;
}
