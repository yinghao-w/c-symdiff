#include "symbols.h"

#define DPX_KT char
#define DPX_VT Opr
#include "dpx.h"

DPX_map *DPX_opr_set;

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
  DPX_opr_set = DPX_create(1);
  DPX_add('+', (Opr){"+", 2, 1, add}, DPX_opr_set);
  DPX_add('-', (Opr){"-", 2, 1, sub}, DPX_opr_set);
  DPX_add('*', (Opr){"*", 2, 2, mul}, DPX_opr_set);
  DPX_add('/', (Opr){"/", 2, 2, divi}, DPX_opr_set);
  DPX_add('^', (Opr){"^", 2, 3, NULL}, DPX_opr_set);
  DPX_add('@', (Opr){"@", 1, 4, NULL}, DPX_opr_set);
  DPX_add('(', (Opr){"(", 0, 0, NULL}, DPX_opr_set);
  DPX_add(')', (Opr){")", 0, 0, NULL}, DPX_opr_set);
}

Opr *opr_get(const char s) { return DPX_addr(s, DPX_opr_set); }

int opr_cmp(const Opr *opr1, const Opr *opr2) {
  if (opr1->precedence > opr2->precedence) {
    return 1;
  } else if (opr1->precedence < opr2->precedence) {
    return -1;
  } else {
    return 0;
  }
}
