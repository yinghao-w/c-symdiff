#ifndef AST_H

#define AST_H

#include "symbols.h"

#define T_TYPE Token
#define T_PREFIX ast
#define T_STRUCT_PREFIX Ast
#include "tree.h"

Ast_Node *ast_create(char expr[]);

int ast_expr_is_equal(Ast_Node *expr1, Ast_Node *expr2);

Ast_Node *ast_copy(Ast_Node *expr);

#endif
