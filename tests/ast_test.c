#include "ast.h"
#define T_DEBUG
#include "ast.c"
#include "lexer.h"
#include "symbols.h"
#include <stdio.h>

#define NUM_EXPRS 3

void opr_set_setup(void) { opr_set_init(); }

struct test_exprs_formats {
  char s[32];
  Token tokens[16];
  Ast_Node *tree;
};

struct test_exprs_formats test_exprs_all[NUM_EXPRS];

void token_print(Token token) {
  switch (token.token_type) {
  case SCALAR:
    printf("%.f", token.scalar);
    break;
  case VAR:
    printf("%c", token.var);
    break;
  case OPR:
    printf("%c", token.opr->repr[0]);
    break;
  }
}

#define ASSERT_TOKEN_EQUAL(token1, token2)                                     \
  do {                                                                         \
    if (!tok_cmp(token1, token2)) {                                            \
      printf("Assertion failed: %s == %s\n", #token1, #token2);                \
      printf("In file %s, function %s, line %d\n", __FILE__, __func__,         \
             __LINE__);                                                        \
      printf("%s: ", #token1);                                                 \
      token_print(token1);                                                     \
      printf("\n%s: ", #token2);                                               \
      token_print(token2);                                                     \
      printf("\n");                                                            \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define VAR_OPR_TOKENS_SETUP()                                                 \
  Token add = {.token_type = OPR};                                             \
  add.opr = opr_get('+');                                                      \
  Token sub = {.token_type = OPR};                                             \
  sub.opr = opr_get('-');                                                      \
  Token mul = {.token_type = OPR};                                             \
  mul.opr = opr_get('*');                                                      \
  Token divi = {.token_type = OPR};                                            \
  divi.opr = opr_get('/');                                                     \
  Token pow = {.token_type = OPR};                                             \
  pow.opr = opr_get('^');                                                      \
  Token exp = {.token_type = OPR};                                             \
  exp.opr = opr_get('@');                                                      \
  Token lp = {.token_type = OPR};                                              \
  lp.opr = opr_get('(');                                                       \
  Token rp = {.token_type = OPR};                                              \
  rp.opr = opr_get(')');                                                       \
                                                                               \
  Token x = {.token_type = VAR};                                               \
  x.var = 'x';                                                                 \
  Token b = {.token_type = VAR};                                               \
  b.var = 'b';                                                                 \
  Token c = {.token_type = VAR};                                               \
  c.var = 'c';

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
      ast_join(mul,

               ast_leaf((Token){SCALAR, {3.0}}),

               ast_join(add,

                        ast_leaf(x),

                        ast_leaf((Token){SCALAR, {2.0}})

                            )

                   )

  };

  test_exprs_all[1] =
      (struct test_exprs_formats){"1.5/ (b + @( c ))",
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
                                  ast_join(divi,

                                           ast_leaf((Token){SCALAR, {1.5}}),

                                           ast_join(add,

                                                    ast_leaf(b),

                                                    ast_join(exp,

                                                             ast_leaf(c),

                                                             NULL

                                                             )

                                                        )

                                               )

      };

  test_exprs_all[2] = (struct test_exprs_formats){
      "@ x - -7.8 * x",
      {
          exp,
          x,
          sub,
          (Token){SCALAR, {-7.8}},
          mul,
          x,
      },
      ast_join(sub,

               ast_join(exp,

                        ast_leaf(x), NULL),

               ast_join(mul,

                        ast_leaf((Token){SCALAR, {-7.8}}),

                        ast_leaf(x)

                            )

                   )

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

void test_ast_expr_is_equal(void) {
  char s[] = "  @(7- 5.2)";
  char t[] = "@ ( 7 - 5.2) ";
  Ast_Node *ast_s = ast_create(s);
  Ast_Node *ast_t = ast_create(t);
  assert(ast_expr_is_equal(ast_s, ast_t));

  char u[] = "@ ( 7 / 5.2) ";
  Ast_Node *ast_u = ast_create(u);
  assert(!ast_expr_is_equal(ast_s, ast_u));

  char a[] = "1 + x + 3";
  char b[] = "1 + 3 + x";
  char c[] = "1 + (x + 3)";
  Ast_Node *ast_a = ast_create(a);
  Ast_Node *ast_b = ast_create(b);
  Ast_Node *ast_c = ast_create(c);
  assert(!ast_expr_is_equal(ast_a, ast_b));
  assert(!ast_expr_is_equal(ast_a, ast_c));

  ast_destroy(ast_s);
  ast_destroy(ast_t);
  ast_destroy(ast_u);
  ast_destroy(ast_a);
  ast_destroy(ast_b);
  ast_destroy(ast_c);
  printf("%s passed\n", __func__);
}

void test_shunting_yard(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Ast_Node *expr = shunting_yard(lexer(test_exprs_all[i].s));
    Ast_Node *expected = test_exprs_all[i].tree;
    assert(ast_expr_is_equal(expr, expected));
	ast_destroy(expr);
  }
  printf("%s passed\n", __func__);
}

void test_ast_create(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Ast_Node *expr = ast_create(test_exprs_all[i].s);
    Ast_Node *expected = test_exprs_all[i].tree;
    assert(ast_expr_is_equal(expr, expected));
	ast_destroy(expr);
  }
  printf("%s passed\n", __func__);
}

void test_ast_copy(void) {
  for (int i = 0; i < NUM_EXPRS; i++) {
    Ast_Node *copy = ast_copy(test_exprs_all[i].tree);
    assert(ast_expr_is_equal(copy, test_exprs_all[i].tree));
	ast_destroy(copy);
  }
  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_setup();
  test_exprs_setup();
  test_lexer_2();
  test_shunting_yard();
  test_ast_create();
  test_ast_expr_is_equal();
  test_tok_cmp();
  test_ast_copy();
  test_exprs_teardown();
}

int main(void) {
  run_tests();
  return 0;
}
