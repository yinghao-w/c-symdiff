#include "symbols.h"
#include <math.h>
#include <stdio.h>

#define DPX_KT char
#define DPX_VT Opr
#define DPX_PFX op
#define DPX_STRUCT_PFX Op
#include "dpx.h"

OpMap *opr_set;

static float add(float args[]) { return args[0] + args[1]; }
static float sub(float args[]) { return args[0] - args[1]; }
static float mul(float args[]) { return args[0] * args[1]; }
static float divi(float args[]) { return args[0] / args[1]; }

static float powe(float args[]) { return pow(args[0], args[1]); }
static float expo(float args[]) { return exp(args[0]); }

/* @(x) is exp(x),
 * x'y is (d/dx)(y) */
void opr_set_init(void) {
  opr_set = op_create(1);
  op_add('+', (Opr){"+", 2, 1, add}, opr_set);
  op_add('-', (Opr){"-", 2, 1, sub}, opr_set);
  op_add('*', (Opr){"*", 2, 2, mul}, opr_set);
  op_add('/', (Opr){"/", 2, 2, divi}, opr_set);
  op_add('^', (Opr){"^", 2, 3, powe}, opr_set);
  op_add('@', (Opr){"@", 1, 4, expo}, opr_set);
  op_add('\'', (Opr){"\'", 2, 5, NULL}, opr_set);
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

int tok_cmp(Token t1, Token t2) {
  if (t1.token_type != t2.token_type) {
    return 0;
  } else {
    switch (t1.token_type) {
    case SCALAR:
      return t1.scalar - t2.scalar < 0.01;
      break;
    case VAR:
      return t1.var == t2.var;
      break;
    case OPR:
      return t1.opr == t2.opr;
      break;
    }
  }
}

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
