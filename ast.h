#ifndef AST_H

#define AST_H

#include "symbols.h"

/* TODO: use opaque pointers */
typedef struct Expression Expression;

Expression expr_create(char expr[]);
void expr_destroy(Expression expr);
Expression expr_copy(Expression expr);
int expr_is_equal(Expression expr1, Expression expr2);

/* Initialise expression transforms. */
void simpls_init(void);
void norm_rules_init(void);
void diff_rules_init(void);

/* Apply normalisation and differentiation transforms. */
int norm_apply(Expression expr);
int diff_apply(Expression expr);

void expr_print(Expression expr);

#endif
