#include "ast.h"
#include "transforms.c"
#include <assert.h>
#include <stdio.h>

// struct test_expr {
// 	char str[32];
// 	int eval;
// 	Expression *simpl;
// };
//
// #define NUM_EXPRS 3
//
// struct test_expr test_exprs[NUM_EXPRS];
//
// void test_exprs_setup(void) {
//   test_exprs[0] = (struct test_expr){
//       "4 - 9",
//       3,
//       NULL,
//   };
//   test_exprs[0] = (struct test_expr){
//       "9 * 8",
//       72,
//       NULL,
//   };
//   test_exprs[0] = (struct test_expr){
//       "11 * ",
//       72,
//       NULL,
//   };
// }

void test_eval(void) {
  Expression *expr1 = expr_create("4 - 9");
  Expression *expr2 = expr_create("2 / 3.9 - 4");
  Expression *expr3 = expr_create("1.1 + 5");
  struct ctx_base base;

  eval(expr1->ast_tree, &base);
  assert(expr1->ast_tree->value.scalar == -5);
  assert(!expr1->ast_tree->lchild);
  eval(expr2->ast_tree, &base);
  assert(expr2->ast_tree->value.token_type == OPR);
  eval(expr3->ast_tree, &base);
  assert(expr3->ast_tree->value.scalar - 6.1 < 0.01);
  assert(!expr1->ast_tree->rchild);

  expr_destroy(expr1);
  expr_destroy(expr2);
  expr_destroy(expr3);
  printf("%s passed\n", __func__);
}

void test_id_apply(void) {
  Expression *expr1 = expr_create("0 + x");
  Expression *expr2 = expr_create("(5 - @ x)* 1");
  struct ctx_simpl add_0_ctx = {{expr1, 0}, opr_get('+'), 0};
  struct ctx_simpl mul_0_ctx = {{expr2, 0}, opr_get('*'), 1};
  Expression *expected_expr1 = expr_create("x");
  Expression *expected_expr2 = expr_create("5 - @ x");

  id_apply(expr1->ast_tree, &add_0_ctx);
  id_apply(expr1->ast_tree, &mul_0_ctx);
  assert(expr_is_equal(expr1, expected_expr1));
  assert(expr_is_equal(expr2, expected_expr2));

  expr_destroy(expr1);
  expr_destroy(expr2);
  expr_destroy(expected_expr1);
  expr_destroy(expected_expr2);
  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_init();

  test_eval();
  test_id_apply();
}

int main(void) {
  run_tests();
  return 0;
}
