#ifndef AST_H

#define AST_H

#include "symbols.h"

#define T_TYPE Token
#define T_PREFIX ast
#define T_STRUCT_PREFIX Ast
#include "tree.h"

typedef struct {
  Ast_Node *ast_tree;
  Ast_Node *dummy_parent;
} Expression;

Ast_Node *get_root(Expression *expr);
void set_root(Ast_Node *root, Expression *expr);


Expression *expr_create(char expr[]);
void expr_destroy(Expression *expr);
int expr_is_equal(Expression *expr1, Expression *expr2);
Expression *expr_copy(Expression *expr);

#endif
