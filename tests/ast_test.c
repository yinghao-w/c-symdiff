#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.c"
#include "lexer.h"
#include "symbols.h"
#include <assert.h>
#include <stdio.h>

void opr_set_setup(void) { opr_set_init(); }

#define ASSERT_TOKEN_EQUAL(token1, token2)                                     \
  do {                                                                         \
    if (!tok_is_equal(token1, token2)) {                                       \
      printf("Assertion failed: %s == %s\n", #token1, #token2);                \
      printf("In file %s, function %s, line %d\n", __FILE__, __func__,         \
             __LINE__);                                                        \
      printf("%s: ", #token1);                                                 \
      tok_print(token1);                                                       \
      printf("\n%s: ", #token2);                                               \
      tok_print(token2);                                                       \
      printf("\n");                                                            \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define VAR_OPR_TOKENS_SETUP()                                                 \
  Token add = {.token_type = OPR};                                             \
  add.opr = opr_get("+");                                                      \
  Token sub = {.token_type = OPR};                                             \
  sub.opr = opr_get("-");                                                      \
  Token mul = {.token_type = OPR};                                             \
  mul.opr = opr_get("*");                                                      \
  Token divi = {.token_type = OPR};                                            \
  divi.opr = opr_get("/");                                                     \
  Token exp = {.token_type = OPR};                                             \
  exp.opr = opr_get("exp");                                                    \
  Token lp = {.token_type = OPR};                                              \
  lp.opr = opr_get("(");                                                       \
  Token rp = {.token_type = OPR};                                              \
  rp.opr = opr_get(")");                                                       \
                                                                               \
  Token x = {.token_type = VAR};                                               \
  x.var = 'x';                                                                 \
  Token b = {.token_type = VAR};                                               \
  b.var = 'b';                                                                 \
  Token c = {.token_type = VAR};                                               \
  c.var = 'c';                                                                 \
                                                                               \
  Token dummy = {.token_type = VAR};                                           \
  dummy.var = '#';

struct test_exprs_formats {
  char s[32];
  Token tokens[16];
  Ast_Node *tree;
};

#define NUM_EXPRS 3

struct test_exprs_formats test_exprs_all[NUM_EXPRS];

void test_exprs_setup(void) {
  VAR_OPR_TOKENS_SETUP();

  test_exprs_all[0] = (struct test_exprs_formats){
      "3 *(x + 2)",
      {
          (Token){SCALAR, {3.0}},
          mul,
          lp,
          x,
          add,
          (Token){SCALAR, {2.0}},
          rp,
      },
      ast_join(dummy,
               ast_join(mul,

                        ast_leaf((Token){SCALAR, {3.0}}),

                        ast_join(add,

                                 ast_leaf(x),

                                 ast_leaf((Token){SCALAR, {2.0}})

                                     )

                            ),
               NULL)

  };

  test_exprs_all[1] = (struct test_exprs_formats){
      "1.5/ (b + exp( c ))",
      {
          (Token){SCALAR, {1.5}},
          divi,
          lp,
          b,
          add,
          exp,
          lp,
          c,
          rp,
          rp,
      },
      ast_join(dummy,
               ast_join(divi,

                        ast_leaf((Token){SCALAR, {1.5}}),

                        ast_join(add,

                                 ast_leaf(b),

                                 ast_join(exp,

                                          ast_leaf(c),

                                          NULL

                                          )

                                     )

                            ),
               NULL)

  };

  test_exprs_all[2] = (struct test_exprs_formats){
      "exp x - -7.8 * x",
      {
          exp,
          x,
          sub,
          (Token){SCALAR, {-7.8}},
          mul,
          x,
      },
      ast_join(dummy,
               ast_join(sub,

                        ast_join(exp,

                                 ast_leaf(x), NULL),

                        ast_join(mul,

                                 ast_leaf((Token){SCALAR, {-7.8}}),

                                 ast_leaf(x)

                                     )

                            ),
               NULL)

  };
}

void test_exprs_teardown(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    ast_destroy(test_exprs_all[i].tree);
  }
}

void test_build(void) { ; }

void test_tok_cmp(void) { ; }

void test_lexer_2(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Token *tokens = lexer(test_exprs_all[0].s);
    for (int j = 0; j < fp_length(tokens); j++) {
      ASSERT_TOKEN_EQUAL(tokens[j], test_exprs_all[0].tokens[j]);
    }
    fp_destroy(tokens);
  }

  printf("%s passed\n", __func__);
}

void test_expr_is_equal(void) {
  Expression ast_s = expr_create("  exp(7- 5.2)");
  Expression ast_t = expr_create("exp ( 7 - 5.2) ");
  assert(expr_is_equal(ast_s, ast_t));

  Expression ast_u = expr_create("exp (7 / 5.2)  ");
  assert(!expr_is_equal(ast_s, ast_u));

  Expression ast_a = expr_create("1 + x + 3");
  Expression ast_b = expr_create("1 + 3 + x");
  Expression ast_c = expr_create("1 + (x + 3)");

  assert(!expr_is_equal(ast_a, ast_b));
  assert(!expr_is_equal(ast_a, ast_c));

  expr_destroy(ast_s);
  expr_destroy(ast_t);
  expr_destroy(ast_u);
  expr_destroy(ast_a);
  expr_destroy(ast_b);
  expr_destroy(ast_c);
  printf("%s passed\n", __func__);
}

void test_shunting_yard(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Ast_Node *expr = shunting_yard(lexer(test_exprs_all[i].s));
    Ast_Node *expected = test_exprs_all[i].tree->lchild;
    assert(ast_is_equal(expr, expected, tok_is_equal));
    ast_destroy(expr);
  }

  printf("%s passed\n", __func__);
}

void test_expr_create(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Expression expr = expr_create(test_exprs_all[i].s);
    Expression expected = {test_exprs_all[i].tree};
    assert(expr_is_equal(expr, expected));
    expr_destroy(expr);
  }

  printf("%s passed\n", __func__);
}

void test_ast_copy(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Ast_Node *original = test_exprs_all[i].tree;
    Ast_Node *copy = ast_copy(original);
    assert(ast_is_equal(copy, original, tok_is_equal));
    ast_destroy(copy);
  }

  printf("%s passed\n", __func__);
}



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
  Expression expr2 = expr_create("(5 - exp x)* 1");
  struct CtxAll add_0_ctx = {0, simpls};
  struct CtxAll mul_1_ctx = {0, simpls + 1};

  Expression expected_expr1 = expr_create("x");
  Expression expected_expr2 = expr_create("5 - exp x");

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
  assert(ast_is_equal(bind_get('f', bindings), get_root(expr)->lchild,
                      tok_is_equal));

  expr_destroy(pattern);
  expr_destroy(expr);
  bind_destroy(bindings);
  printf("%s passed\n", __func__);
}

void test_match_apply(void) {
  Expression expr = expr_create("x - (exp x)");
  Expression expected = expr_create("x + (-1 * exp x)");
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
  opr_set_setup();
  test_exprs_setup();

  test_lexer_2();
  test_shunting_yard();
  test_expr_create();
  test_expr_is_equal();
  test_tok_cmp();
  test_ast_copy();

  test_exprs_teardown();


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
