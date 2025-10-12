#include "ast.h"
#include "../c-generics/fat_pointer.h"
#include "lexer.h"
#include <assert.h>

#define fp_peek(darray) darray[fp_length(darray) - 1]

/* Takes an operator and builds a tree node with value operator, and arity
 * number of children from the top of the out stack. Pushes the new node onto
 * the out stack. */
static void build(Ast_Node *out[], Token *oprs) {
  Token opr = fp_pop(oprs);
  Ast_Node *node = NULL;
  if (opr.opr->arity == 1) {
    node = ast_join(opr, fp_pop(out), NULL);
  } else if (opr.opr->arity == 2) {
    Ast_Node *rchild = fp_pop(out);
    Ast_Node *lchild = fp_pop(out);
    node = ast_join(opr, lchild, rchild);
  }
  fp_push(node, out);
}

/* Shunting yard algorithm */
static Ast_Node *shunting_yard(Token tokens[]) {
  /* Initialises operator stack and output stack. Output stack consists of nodes
   * and should be at most 2 elements always? */
  Token *oprs = NULL;
  Ast_Node **out = NULL;

  for (size_t i = 0; i < fp_length(tokens); i++) {
    Token token = tokens[i];

    if (token.token_type != OPR) {
      Ast_Node *node = ast_leaf(token);
      fp_push(node, out);

    } else if (token.token_type == OPR) {
      if (token.opr->arity == 1) {
        fp_push(token, oprs);

      } else if (token.opr->arity == 2) {
        while ((fp_length(oprs) > 0) && !(fp_peek(oprs).opr->repr[0] == '(') &&
               (opr_cmp(fp_peek(oprs).opr, token.opr) >= 0)) {
          build(out, oprs);
        }
        fp_push(token, oprs);

      } else if (token.opr->repr[0] == '(') {
        fp_push(token, oprs);

      } else if (token.opr->repr[0] == ')') {
        assert(fp_length(oprs) > 0);
        while (fp_peek(oprs).opr->repr[0] != '(') {
          build(out, oprs);
        }
        assert(fp_peek(oprs).opr->repr[0] == '(');
        (void)fp_pop(oprs);

        if ((fp_length(oprs) > 0) && (fp_peek(oprs).opr->arity == 1)) {
          build(out, oprs);
        }
      }
    }
  }
  fp_destroy(tokens);

  while (fp_length(oprs) > 0) {
    assert(fp_peek(oprs).opr->repr[0] != '(');
    build(out, oprs);
  }
  fp_destroy(oprs);
  Ast_Node *root = fp_pop(out);
  fp_destroy(out);

  return root;
}

Expression expr_create(char expr[]) {
  Expression *p = malloc(sizeof(*p));
  Ast_Node *ast_tree = shunting_yard(lexer(expr));

  /* Give the AST root a dummy parent to simplify tree modification functions */
  Token token;
  token.token_type = VAR;
  token.var = '#';
  p->dummy_parent = ast_join(token, ast_tree, NULL);

  return *p;
}

void expr_destroy(Expression expr) { ast_destroy(expr.dummy_parent); }

Ast_Node *get_root(Expression expr) { return expr.dummy_parent->lchild; }
void set_root(Ast_Node *root, Expression expr) {
  ast_detach(expr.dummy_parent->lchild);
  ast_attach(root, expr.dummy_parent);
}

int expr_is_equal(Expression expr1, Expression expr2) {
  return ast_is_equal(expr1.dummy_parent, expr2.dummy_parent, tok_cmp);
}

Expression expr_copy(Expression expr) {
  Ast_Node *tree = expr.dummy_parent;
  Ast_Node *copy_root = ast_leaf(tree->value);
  Ast_Iter *it1 = ast_iter_create(tree, T_PRE);
  Ast_Iter *it2 = ast_iter_create(copy_root, T_PRE);
  for (ast_start(it1), ast_start(it2), ast_traverse(it1); !ast_end(it1);
       ast_traverse(it1)) {
    switch (it1->dir) {
    case LCHILD:
    case RCHILD:
      break;
    case LPARENT:
      if (!it2->head->lchild) {
        it2->head->lchild = ast_leaf(it1->head->value);
        it2->head->lchild->parent = it2->head;
      }
      break;
    case RPARENT:
      if (!it2->head->rchild) {
        it2->head->rchild = ast_leaf(it1->head->value);
        it2->head->rchild->parent = it2->head;
      }
      break;
    }
    ast_traverse(it2);
  }
  free(it1);
  free(it2);

  Expression *p = malloc(sizeof(*p));
  p->dummy_parent = copy_root;
  return *p;
}

Ast_Node *ast_copy(Ast_Node *expr) {
  Ast_Node *copy_root = ast_leaf(expr->value);
  Ast_Iter *it1 = ast_iter_create(expr, T_PRE);
  Ast_Iter *it2 = ast_iter_create(copy_root, T_PRE);
  for (ast_start(it1), ast_start(it2), ast_traverse(it1); !ast_end(it1);
       ast_traverse(it1)) {
    switch (it1->dir) {
    case LCHILD:
    case RCHILD:
      break;
    case LPARENT:
      if (!it2->head->lchild) {
        it2->head->lchild = ast_leaf(it1->head->value);
        it2->head->lchild->parent = it2->head;
      }
      break;
    case RPARENT:
      if (!it2->head->rchild) {
        it2->head->rchild = ast_leaf(it1->head->value);
        it2->head->rchild->parent = it2->head;
      }
      break;
    }
    ast_traverse(it2);
  }
  free(it1);
  free(it2);

  return copy_root;
}
