#ifndef TRANSFORMS_H

#define TRANSFORMS_H

#include "ast.h"

int norm_apply(Expression expr);
int diff_apply(Expression expr);

void simpls_init(void);
void rules_init(void);
void diff_rules_init(void);

#endif
