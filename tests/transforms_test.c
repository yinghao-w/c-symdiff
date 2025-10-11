#include "ast.h"
#include "symbols.h"
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

void test_eval_apply(void) {
  Expression *expr1 = expr_create("4 - 9");
  Expression *expr2 = expr_create("2 / 3.9 - 4");
  Expression *expr3 = expr_create("1.1 + 5");
  struct CtxAll ctx = {expr1, 0, NULL};

  eval_apply(expr1->ast_tree, &ctx);
  assert(expr1->ast_tree->value.scalar == -5);
  assert(!expr1->ast_tree->lchild);
  eval_apply(expr2->ast_tree, &ctx);
  assert(expr2->ast_tree->value.token_type == OPR);
  eval_apply(expr3->ast_tree, &ctx);
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
  struct CtxAll add_0_ctx = {expr1, 0, simpls};
  struct CtxAll mul_1_ctx = {expr2, 0, simpls + 1};

  Expression *expected_expr1 = expr_create("x");
  Expression *expected_expr2 = expr_create("5 - @ x");

  id_apply(expr1->ast_tree, &add_0_ctx);
  id_apply(expr2->ast_tree, &mul_1_ctx);
  assert(expr_is_equal(expr1, expected_expr1));
  assert(expr_is_equal(expr2, expected_expr2));

  Expression *expr3 = expr_create("(x + 0) / y");
  struct CtxAll add_0_ctx2 = {expr3, 0, simpls};
  Expression *expected_expr3 = expr_create("x / y");
  id_apply(expr3->ast_tree->lchild, &add_0_ctx2);

  assert(expr_is_equal(expr3, expected_expr3));

  expr_destroy(expr1);
  expr_destroy(expr2);
  expr_destroy(expr3);
  expr_destroy(expected_expr1);
  expr_destroy(expected_expr2);
  expr_destroy(expected_expr3);
  printf("%s passed\n", __func__);
}

void test_absorp_apply(void) {
  Expression *expr = expr_create("(3 * 0) * (2 * a - 4)");
  struct CtxAll mul_0_ctx = {expr, 0, simpls + 2};
  Expression *expected_expr1 = expr_create("0 * (2 *a - 4 )");
  Expression *expected_expr2 = expr_create("0");

  absorp_apply(expr->ast_tree->lchild, &mul_0_ctx);
  assert(expr_is_equal(expr, expected_expr1));
  absorp_apply(expr->ast_tree, &mul_0_ctx);
  assert(expr_is_equal(expr, expected_expr2));

  expr_destroy(expr);
  expr_destroy(expected_expr1);
  expr_destroy(expected_expr2);
  printf("%s passed\n", __func__);
}

void test_var_match(void) {
  Expression *expr = expr_create("3 ^ y");

  assert(var_match('f', expr->ast_tree));
  assert(!var_match('c', expr->ast_tree));
  assert(var_match('c', expr->ast_tree->lchild));

  expr_destroy(expr);
  printf("%s passed\n", __func__);
}

void test_match(void) {
  Expression *pattern = expr_create("f + 2");
  Expression *expr = expr_create("(-5 / y) + 2");

  BindMap *bindings = bind_create(1);
  assert(match(pattern->ast_tree, expr->ast_tree, bindings));
  assert(bind_size(bindings) == 1);
  assert(bind_is_in('f', bindings));
  assert(
      ast_is_equal(bind_get('f', bindings), expr->ast_tree->lchild, tok_cmp));

  expr_destroy(pattern);
  expr_destroy(expr);
  bind_destroy(bindings);
  printf("%s passed\n", __func__);
}

void test_match_apply(void) {
  Expression *expr = expr_create("x - (@ x)");
  Expression *expected = expr_create("x + (-1 * @ x)");
  struct CtxAll ctx = {expr, 0, rules};

  match_apply(expr->ast_tree, &ctx);
  assert(ctx.changed = 1);
  assert(ast_is_equal(expr->ast_tree, expected->ast_tree, tok_cmp));
  assert(ast_is_equal(expected->ast_tree, expr->ast_tree, tok_cmp));

  expr_destroy(expr);
  expr_destroy(expected);

  expr = expr_create("(a/b) + 0");
  expected = expr_create("a * b^(-1) + 0");
  ctx.expr = expr;
  ctx.ctx_trans = rules + 1;

  match_apply(expr->ast_tree->lchild, &ctx);
  assert(ast_is_equal(expr->ast_tree, expected->ast_tree, tok_cmp));

  expr_destroy(expr);
  expr_destroy(expected);

  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_init();

  test_eval_apply();

  simpls_init();

  test_id_apply();
  test_absorp_apply();

  rules_init();

  test_var_match();
  test_match();
  test_match_apply();
}

int main(void) {
  run_tests();
  return 0;
}
