#ifndef SYMBOLS_H

#define SYMBOLS_H

typedef struct {
  char repr[4];
  int arity;
  int precedence;
  float (*func)(float *args);
} Opr;

/* Initialise and cleanup global operator definitions. */
void opr_set_init(void);
void opr_set_cleanup(void);
Opr *opr_get(const char s);

/* Return 1 if opr1 is higher precedence than opr2, -1 if opr is lower
 * precedence, and 0 if equal, i.e. >  */
int opr_cmp(const Opr *opr1, const Opr *opr2);

typedef float Scalar;
typedef char Var;

typedef enum { SCALAR, VAR, OPR } TOKEN_TYPE;
typedef struct {
  TOKEN_TYPE token_type;
  union {
    Scalar scalar;
    Var var;
    Opr *opr;
  };
} Token;

int tok_is_equal(Token token1, Token token2);

#ifdef SYMBOLS_DEBUG
void tok_print(Token token);
#endif

#endif
