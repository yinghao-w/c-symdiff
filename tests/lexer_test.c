#include "../lexer.c"
#include "../lexer.h"
#include <assert.h>
#include <stdio.h>

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

void test_opr_match(void) { ; }

void test_match(void) { ; }

void test_lexer(void) { ; }

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
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
