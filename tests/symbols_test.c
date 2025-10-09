#include "symbols.c"
#include <assert.h>
#include <stdio.h>

extern DPX_map *DPX_opr_set;

void test_opr_set_init(void) {
  opr_set_init();
  DPX_bucket *buckets = DPX_opr_set->data;
  assert(buckets[0].key == '+');
  assert(buckets[4].key == '^');
  assert(buckets[3].value.arity == 2);
  assert(buckets[1].value.repr[0] == '-');
  printf("%s passed\n", __func__);
}

void test_opr_get(void) {
  Opr *add = opr_get('+');
  assert(add->repr[0] == '+');
  assert(add->arity == 2);
  assert(opr_get('q') == NULL);
  printf("%s passed\n", __func__);
}

void test_opr_cmp(void) {
  assert(opr_cmp(opr_get('/'), opr_get('-')) == 1);
  assert(opr_cmp(opr_get('+'), opr_get('-')) == 0);
  assert(opr_cmp(opr_get('+'), opr_get('*')) == -1);
  printf("%s passed\n", __func__);
}

void run_tests(void) {
  printf("\n\n%s\n\n", __FILE__);
  test_opr_set_init();
  test_opr_get();
  test_opr_cmp();
}

int main(void) {
  run_tests();
  return 0;
}
