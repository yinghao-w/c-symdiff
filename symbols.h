#ifndef SYMBOLS_H

#define SYMBOLS_H

typedef float Scalar;
typedef char Var;

typedef struct {
  char repr[4];
  int arity;
  float (*func)(float *args);
} Opr;

typedef Opr ms_set[];

int in_set(const ms_set);
Opr *get_opr(const char s[]);

/* Return 1 if opr1 is higher precedence than opr2, -1 if opr is lower
 * precedence, and 0 if equal, i.e. >  */
int oprcmp(const Opr *opr1, const Opr *opr2);

typedef enum { SCALAR, VAR, OPR } TOKEN_TYPE;

typedef struct {
  TOKEN_TYPE token_type;
  union {
    Scalar scalar;
    Var var;
    Opr *opr;
  };
} Token;

#endif
