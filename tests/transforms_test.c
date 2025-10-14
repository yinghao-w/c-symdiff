#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.h"
#include "symbols.h"
#include "transforms.c"
#include <assert.h>
#include <stdio.h>

void test_eval_apply(void) {
  Expression expr1 = expr_create("4 - 9");
  Expression expr2 = expr_create("2 / 3.9 - 4");
  Expression expr3 = expr_create("1.1 + 5");
  struct CtxAll ctx = {0, NULL};

  eval_apply(get_root(expr1), &ctx);
  assert(get_root(expr1)->value.scalar == -5);
  assert(!get_root(expr1)->lchild);
  eval_apply(get_root(expr2), &ctx);
  assert(get_root(expr2)->value.token_type == OPR);
  eval_apply(get_root(expr3), &ctx);
  assert(get_root(expr3)->value.scalar - 6.1 < 0.01);
  assert(!get_root(expr1)->rchild);

  expr_destroy(expr1);
  expr_destroy(expr2);
  expr_destroy(expr3);
  printf("%s passed\n", __func__);
}

void test_id_apply(void) {
  Expression expr1 = expr_create("0 + x");
  Expression expr2 = expr_create("(5 - @ x)* 1");
  struct CtxAll add_0_ctx = {0, simpls};
  struct CtxAll mul_1_ctx = {0, simpls + 1};

  Expression expected_expr1 = expr_create("x");
  Expression expected_expr2 = expr_create("5 - @ x");

  id_apply(get_root(expr1), &add_0_ctx);
  id_apply(get_root(expr2), &mul_1_ctx);
  assert(expr_is_equal(expr1, expected_expr1));
  assert(expr_is_equal(expr2, expected_expr2));

  Expression expr3 = expr_create("(x + 0) / y");
  struct CtxAll add_0_ctx2 = {0, simpls};
  Expression expected_expr3 = expr_create("x / y");
  id_apply(get_root(expr3)->lchild, &add_0_ctx2);

  assert(expr_is_equal(expr3, expected_expr3));

  expr_destroy(expr1);
  expr_destroy(expr2);
  expr_destroy(expr3);
  expr_destroy(expected_expr1);
  expr_destroy(expected_expr2);
  expr_destroy(expected_expr3);
  printf("%s passed\n", __func__);
}

void test_ann_apply(void) {
  Expression expr = expr_create("(3 * 0) * (2 * a - 4)");
  struct CtxAll mul_0_ctx = {0, simpls + 2};
  Expression expected_expr1 = expr_create("0 * (2 *a - 4 )");
  Expression expected_expr2 = expr_create("0");

  ann_apply(get_root(expr)->lchild, &mul_0_ctx);
  assert(expr_is_equal(expr, expected_expr1));
  ann_apply(get_root(expr), &mul_0_ctx);
  assert(expr_is_equal(expr, expected_expr2));

  expr_destroy(expr);
  expr_destroy(expected_expr1);
  expr_destroy(expected_expr2);
  printf("%s passed\n", __func__);
}

void test_var_match(void) {
  Expression expr = expr_create("3 ^ y");

  assert(var_match('f', get_root(expr)));
  assert(!var_match('c', get_root(expr)));
  assert(var_match('c', get_root(expr)->lchild));

  expr_destroy(expr);
  printf("%s passed\n", __func__);
}

void test_match(void) {
  Expression pattern = expr_create("f + 2");
  Expression expr = expr_create("(-5 / y) + 2");

  BindMap *bindings = bind_create(1);
  assert(match(get_root(pattern), get_root(expr), bindings));
  assert(bind_size(bindings) == 1);
  assert(bind_is_in('f', bindings));
  assert(
      ast_is_equal(bind_get('f', bindings), get_root(expr)->lchild, tok_is_equal));

  expr_destroy(pattern);
  expr_destroy(expr);
  bind_destroy(bindings);
  printf("%s passed\n", __func__);
}

void test_match_apply(void) {
  Expression expr = expr_create("x - (@ x)");
  Expression expected = expr_create("x + (-1 * @ x)");
  struct CtxAll ctx = {0, rules};

  match_apply(get_root(expr), &ctx);
  assert(ctx.changed = 1);
  assert(ast_is_equal(get_root(expr), get_root(expected), tok_is_equal));
  assert(ast_is_equal(get_root(expected), get_root(expr), tok_is_equal));

  expr_destroy(expr);
  expr_destroy(expected);

  expr = expr_create("(a/b) + 0");
  expected = expr_create("a * b^(-1) + 0");
  ctx.ctx_trans = rules + 1;

  match_apply(get_root(expr)->lchild, &ctx);
  assert(ast_is_equal(get_root(expr), get_root(expected), tok_is_equal));

  expr_destroy(expr);
  expr_destroy(expected);

  printf("%s passed\n", __func__);
}

void test_norm_apply(void) {
  Expression expr = expr_create("1 - b/c");
  Expression expected = expr_create("1 + -1 * (b * c ^ -1)");

  assert(norm_apply(expr));
  assert(expr_is_equal(expr, expected));

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
  test_ann_apply();

  rules_init();

  test_var_match();
  test_match();
  test_match_apply();
  test_norm_apply();
}

int main(void) {
  run_tests();
  return 0;
}
