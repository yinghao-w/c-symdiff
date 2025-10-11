#ifndef SYMBOLS_H

#define SYMBOLS_H

typedef float Scalar;
typedef char Var;

typedef struct {
  char repr[4];
  int arity;
  int precedence;
  float (*func)(float *args);
} Opr;

void opr_set_init(void);
Opr *opr_get(const char s);

/* Return 1 if opr1 is higher precedence than opr2, -1 if opr is lower
 * precedence, and 0 if equal, i.e. >  */
int opr_cmp(const Opr *opr1, const Opr *opr2);

typedef enum { SCALAR, VAR, OPR } TOKEN_TYPE;

typedef struct {
  TOKEN_TYPE token_type;
  union {
    Scalar scalar;
    Var var;
    Opr *opr;
  };
} Token;

int tok_cmp(Token t1, Token t2);

#ifdef SYMBOLS_DEBUG
void token_print(Token token);
#endif

#endif
