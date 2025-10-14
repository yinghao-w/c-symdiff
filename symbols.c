#include "symbols.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define DPX_KT char
#define DPX_VT Opr
#define DPX_PFX op
#define DPX_STRUCT_PFX Op
#include "dpx.h"

OpMap *opr_set;

static float add(const float args[]) { return args[0] + args[1]; }
static float sub(const float args[]) { return args[0] - args[1]; }
static float mul(const float args[]) { return args[0] * args[1]; }
static float divi(const float args[]) { return args[0] / args[1]; }

static float powe(const float args[]) { return pow(args[0], args[1]); }
static float expo(const float args[]) { return exp(args[0]); }
static float loga(const float args[]) { return log(args[0]); }
static float sine(const float args[]) { return sin(args[0]); }
static float cosi(const float args[]) { return cos(args[0]); }

/* x'y is (d/dx)(y) */
void opr_set_init(void) {
  opr_set = op_create(1);
  op_add('+', (Opr){"+", 2, 1, add}, opr_set);
  op_add('-', (Opr){"-", 2, 1, sub}, opr_set);
  op_add('*', (Opr){"*", 2, 2, mul}, opr_set);
  op_add('/', (Opr){"/", 2, 2, divi}, opr_set);
  op_add('^', (Opr){"^", 2, 3, powe}, opr_set);

  op_add('e', (Opr){"exp", 1, 4, expo}, opr_set);
  op_add('l', (Opr){"log", 1, 4, loga}, opr_set);
  op_add('s', (Opr){"sin", 1, 4, sine}, opr_set);
  op_add('c', (Opr){"cos", 1, 4, cosi}, opr_set);

  op_add('\'', (Opr){"\'", 2, 5, NULL}, opr_set);
  op_add('(', (Opr){"(", 0, 0, NULL}, opr_set);
  op_add(')', (Opr){")", 0, 0, NULL}, opr_set);

}

void opr_set_cleanup(void) { op_destroy(opr_set); }

Opr *opr_get(const char s[]) {
  Opr *opr = op_addr(s[0], opr_set);
  if (!opr) {
    return NULL;
  } else if (!strncmp(s, opr->repr, REPR_LENGTH)) {
    return opr;
  } else {
    return NULL;
  }
}

int opr_cmp(const Opr *opr1, const Opr *opr2) {
  if (opr1->precedence > opr2->precedence) {
    return 1;
  } else if (opr1->precedence < opr2->precedence) {
    return -1;
  } else {
    return 0;
  }
}

int tok_is_equal(Token token1, Token token2) {
  if (token1.token_type != token2.token_type) {
    return 0;
  } else {
    switch (token1.token_type) {
    case SCALAR:
      /* TODO: add epsilon difference */
      return token1.scalar == token2.scalar;
      break;
    case VAR:
      return token1.var == token2.var;
      break;
    case OPR:
      return token1.opr == token2.opr;
      break;
    }
  }
}

void tok_print(Token token) {
  switch (token.token_type) {
  case SCALAR:
    printf("%.2f", token.scalar);
    break;
  case VAR:
    printf("%c", token.var);
    break;
  case OPR:
    printf("%s", token.opr->repr);
    break;
  }
}
