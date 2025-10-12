#include "lexer.c"
#include <assert.h>
#include <stdio.h>

Scalar epsilon = 0.001;

void opr_set_setup(void) { opr_set_init(); }

void test_scalar_match(void) {
  char s[] = "-13.6";
  assert(!scalar_match(s, s + 1));
  assert(scalar_match(s + 1, s + 2));
  assert(scalar_match(s, s + 5));

  char t[] = "asdiwqd";
  assert(!scalar_match(t, t + 3));

  printf("%s passed\n", __func__);
}

void test_var_match(void) {
  char s[] = "aBKOx";
  assert(var_match(s, s + 3));

  char t[] = "1ci8i9";
  assert(!var_match(t, t + 9));

  printf("%s passed\n", __func__);
}

void test_opr_match(void) {
  char s[] = "(je2";
  assert(opr_match(s, s + 1));
  assert(!opr_match(s, s + 2));
  assert(!opr_match(s + 1, s + 2));

  printf("%s passed\n", __func__);
}

void test_match(void) {
  Token token;
  char s1[] = "12.3ba";
  char *t = s1;
  assert(match(&t, &token) == MATCH_SUCCESS);
  assert(token.token_type == SCALAR);
  assert(token.scalar - 12.3 < epsilon);

  char s2[] = "i+";
  t = s2;
  assert(match(&t, &token) == MATCH_SUCCESS);
  assert(token.token_type == VAR);
  assert(token.var = 'i');

  char s3[] = "*5.9";
  t = s3;
  assert(match(&t, &token) == 0);
  assert(token.token_type == OPR);
  assert(token.opr = opr_get('*'));

  char s4[] = ":99.a";
  t = s4;
  assert(match(&t, &token) == MATCH_ERROR);

  char s5[] = "";
  t = s5;
  assert(match(&t, &token) == MATCH_ERROR);

  printf("%s passed\n", __func__);
}

void test_lexer(void) {
  Token *tokens = lexer(" 1 +3.2/ x  ");

  assert(fp_length(tokens) == 5);
  assert(tokens[0].scalar - 1 < epsilon);
  assert(tokens[1].token_type == OPR);
  assert(tokens[4].var = 'x');
  fp_destroy(tokens);

  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  opr_set_setup();

  test_scalar_match();
  test_var_match();
  test_opr_match();
  test_match();
  test_lexer();
}

int main(void) {
  run_tests();
  return 0;
}
