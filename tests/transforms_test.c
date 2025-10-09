#include "symbols.h"
#include "transforms.c"
#include <assert.h>
#include <stdio.h>

#define TEST_EXPR_SETUP()                                                      \
  char s[] = "(4) - 9";                                                          \
  Ast_Node *expr = ast_create(s);

void test_eval(void) {
  TEST_EXPR_SETUP();
  struct ctx_base base;
  eval(expr, &base);
  assert(expr->value.scalar == -5);
  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_init();
  test_eval();
}

int main(void) {
  run_tests();
  return 0;
}
