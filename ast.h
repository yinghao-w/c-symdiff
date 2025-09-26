#ifndef AST_H

#define AST_H

#include "symbols.h"

#define T_TYPE Token
#define T_PREFIX tok
#include "../c-generics/tree.h"

tok_Node *shunting_yard(char s[]);

#endif
