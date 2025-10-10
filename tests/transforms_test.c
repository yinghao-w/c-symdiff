#include "ast.h"
#include "transforms.c"
#include <assert.h>
#include <stdio.h>

// struct test_expr {
// 	char str[32];
// 	int eval;
// 	Ast_Node *simpl;
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
  Ast_Node *expr1 = ast_create("4 - 9");
  Ast_Node *expr2 = ast_create("2 / 3.9 - 4");
  Ast_Node *expr3 = ast_create("1.1 + 5");
  struct ctx_base base;

  eval(expr1, &base);
  assert(expr1->value.scalar == -5);
  assert(!expr1->lchild);
  eval(expr2, &base);
  assert(expr2->value.token_type == OPR);
  eval(expr3, &base);
  assert(expr3->value.scalar - 6.1 < 0.01);
  assert(!expr1->rchild);

  ast_destroy(expr1);
  ast_destroy(expr2);
  ast_destroy(expr3);

  printf("%s passed\n", __func__);
}

void test_id_apply(void) {
  Ast_Node *expr1 = ast_create("0 + x");
  Ast_Node *expr2 = ast_create("(5 - @ x)* 1");
  struct ctx_simpl add_0_ctx = {{0, 0}, opr_get('+'), 0};
  struct ctx_simpl mul_0_ctx = {{0, 0}, opr_get('*'), 1};
  id_apply(expr1, &add_0_ctx);
  id_apply(expr2, &mul_0_ctx);
  Ast_Node *expected_expr1 = ast_create("x");
  Ast_Node *expected_expr2 = ast_create("5 - @ x");
  assert(ast_expr_is_equal(expr1, expected_expr1));
  assert(ast_expr_is_equal(expr2, expected_expr2));
  ast_destroy(expr1);
  ast_destroy(expr2);
  ast_destroy(expected_expr1);
  ast_destroy(expected_expr2);
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
