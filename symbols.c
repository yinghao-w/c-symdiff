#include "symbols.h"

#define DPX_KT char
#define DPX_VT Opr
#define DPX_PFX op
#define DPX_STRUCT_PFX Op
#include "dpx.h"

OpMap *opr_set;

static float add(float args[]) {
	return args[0] + args[1];
}
static float sub(float args[]) {
	return args[0] - args[1];
}
static float mul(float args[]) {
	return args[0] * args[1];
}
static float divi(float args[]) {
	return args[0] / args[1];
}

void opr_set_init(void) {
  opr_set = op_create(1);
  op_add('+', (Opr){"+", 2, 1, add}, opr_set);
  op_add('-', (Opr){"-", 2, 1, sub}, opr_set);
  op_add('*', (Opr){"*", 2, 2, mul}, opr_set);
  op_add('/', (Opr){"/", 2, 2, divi}, opr_set);
  op_add('^', (Opr){"^", 2, 3, NULL}, opr_set);
  op_add('@', (Opr){"@", 1, 4, NULL}, opr_set);
  op_add('(', (Opr){"(", 0, 0, NULL}, opr_set);
  op_add(')', (Opr){")", 0, 0, NULL}, opr_set);
}

Opr *opr_get(const char s) { return op_addr(s, opr_set); }

int opr_cmp(const Opr *opr1, const Opr *opr2) {
  if (opr1->precedence > opr2->precedence) {
    return 1;
  } else if (opr1->precedence < opr2->precedence) {
    return -1;
  } else {
    return 0;
  }
}
