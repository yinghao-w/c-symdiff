#include "ast.c"
#include "ast.h"
#include "lexer.h"
#include "symbols.h"
#include <stdio.h>

#define NUM_EXPRS 5

void opr_set_setup(void) { opr_set_init(); }

struct test_exprs_formats {
  char s[16];
  Token tokens[8];
  Ast_Node *tree;
};

struct test_exprs_formats test_exprs_all[NUM_EXPRS];

void test_exprs_setup(void) {
  Token x = {.token_type = OPR};
  x.opr = opr_get('-');
  //   test_exprs_all[0] = (struct test_exprs_formats){"3 *(x+ 2) ",
  //                                                   {{SCALAR, {3}},
  //                                                    {OPR, {opr_get('*')}},
  //                                                    {OPR, {opr_get('(')}},
  //                                                    {VAR, {'x'}},
  //                                                    {OPR, {opr_get('+')}},
  //                                                    {SCALAR, {2}},
  //                                                    {OPR, {opr_get(')')}}},
  //                                                   NULL};
}

void test_build(void) {
  ;
}

void test_shunting_yard(void) {
	;
}

void test_ast_create(void) {
  char s[] = "3 *(x+ 2)";
  Ast_Node *expr = ast_create(s);
  assert(expr->value.token_type == OPR);
  assert(expr->lchild->value.scalar - 3 < 0.01);
  assert(expr->rchild->lchild->value.var == 'x');
  printf("%s passed\n", __func__);
}

void test_ast_expr_is_equal(void) { ; }

void test_copy(void);

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_setup();
  test_shunting_yard();
  test_ast_create();
  ;
}

int main(void) {
  run_tests();
  return 0;
}
