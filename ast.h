#ifndef AST_H

#define AST_H

#include "symbols.h"

#define T_TYPE Token
#define T_PREFIX ast
#define T_STRUCT_PREFIX Ast
#include "tree.h"

Ast_Node *ast_create(char expr[]);

#endif
