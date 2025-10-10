#include "ast.h"
#include "symbols.h"
#include <assert.h>

#define T_TYPE(node) (node->value.token_type)
#define T_SCALAR(node) (node->value.scalar)
#define T_VAR(node) (node->value.var)
#define T_OPR(node) (node->value.opr)

#define T_IS_SCALAR(node) (T_TYPE(node) == SCALAR)
#define T_IS_OPR(node) (T_TYPE(node) == OPR)

/* Replaces a new's parent with new. The dummy parent of each AST root allows
 * this to be a replacement, not a overwriting. */
static void ast_replace(Ast_Node *new) {
  assert(new->parent);
  assert(new->parent->parent);
  Ast_Node *parent = new->parent;
  Ast_Node *grandparent = parent->parent;
  ast_detach(parent);
  ast_detach(new);
  ast_destroy(parent);
  /* Assumes that a node will never have a rchild but no lchild. Might need to
   * change below if the assumption changes. */
  if (!grandparent->lchild) {
	  grandparent->lchild = new;
  } else {
	  grandparent->rchild = new;
  }

}

/* Caanged is the output parameter for if a transform or several transforms
 * changed the tree, order is the preferred order to iteratively apply the
 * transform to each node of the tree. Pre-order transforms propagate upwards,
 * post-order transforms propagate downwards. */
struct ctx_base {
  int changed;
  ORDER order;
};

int recursive_apply(Ast_Node *expr, void trans(Ast_Node *, void *), void *ctx) {
  struct ctx_base *base = (struct ctx_base *)ctx;
  ast_iter_apply(expr, base->order, trans, ctx);
  return base->changed;
}

void eval(Ast_Node *expr, void *ctx) {
  struct ctx_base *base = (struct ctx_base *)ctx;
  if (T_IS_OPR(expr)) {
    if (T_OPR(expr)->arity == 1 && T_IS_SCALAR(expr->lchild)) {
      T_TYPE(expr) = SCALAR;
      T_SCALAR(expr) = (T_OPR(expr)->func)(&T_SCALAR(expr->lchild));
      ast_destroy(expr->lchild);
      expr->lchild = NULL;
      base->changed = 1;

    } else if (T_OPR(expr)->arity == 2 && T_IS_SCALAR(expr->lchild) &&
               T_IS_SCALAR(expr->rchild)) {
      T_TYPE(expr) = SCALAR;
      Scalar arr[2] = {T_SCALAR(expr->lchild), T_SCALAR(expr->rchild)};
      T_SCALAR(expr) = (T_OPR(expr)->func)(arr);
      ast_destroy(expr->lchild);
      ast_destroy(expr->rchild);
      expr->lchild = NULL;
      expr->rchild = NULL;
      base->changed = 1;
    }
  }
}

struct ctx_simpl {
  struct ctx_base base;
  Opr *opr;
  Scalar x;
};

void id_apply(Ast_Node *expr, void *ctx) {
  struct ctx_simpl *ctx_simpl = (struct ctx_simpl *)ctx;
  Opr *opr = ctx_simpl->opr;
  Scalar id = ctx_simpl->x;
  ctx_simpl->base.changed = 0;
  if (T_IS_OPR(expr) && T_OPR(expr) == opr) {
    if (T_IS_SCALAR(expr->lchild) && T_SCALAR(expr->lchild) == id) {
      ast_replace(expr->rchild);
      ctx_simpl->base.changed = 1;
    } else if (T_IS_SCALAR(expr->rchild) && T_SCALAR(expr->rchild) == id) {
      ast_replace(expr->lchild);
      ctx_simpl->base.changed = 1;
    }
  }
}

void absorpt_apply(Ast_Node *expr, void *ctx) {
  struct ctx_simpl *ctx_simpl = (struct ctx_simpl *)ctx;
  Opr *opr = ctx_simpl->opr;
  Scalar id = ctx_simpl->x;
  ctx_simpl->base.changed = 0;
  if (T_IS_OPR(expr) && T_OPR(expr) == opr) {
    if (T_IS_SCALAR(expr->lchild) && T_SCALAR(expr->lchild) == id) {
      ast_replace(expr->lchild);
      ctx_simpl->base.changed = 1;
    } else if (T_IS_SCALAR(expr->rchild) && T_SCALAR(expr->rchild) == id) {
      ast_replace(expr->rchild);
      ctx_simpl->base.changed = 1;
    }
  }
}

struct ctx_match {
  struct ctx_base base;
  Ast_Node *pattern;
  Ast_Node *replacement;
};

static int patt_var_match(Var x, Ast_Node *expr) {
  switch (x) {
  case '#':
    return T_IS_SCALAR(expr);
  default:
    return 1;
  }
}

static int patt_match(Ast_Node *patt, Ast_Node *expr,
                      Token **table_placeholder) {
  switch (T_TYPE(patt)) {
  case SCALAR:
    return T_SCALAR(patt) == T_SCALAR(expr);
    break;
  case VAR:
    if (patt_var_match(T_VAR(patt), expr)) {
      *table_placeholder = NULL;
      // set (key, value) to (patt_var, expr)
      return 1;
    } else {
      return 0;
    }
    break;
  case OPR:
    return T_OPR(patt) == T_OPR(expr);
    break;
  }
}

void match_apply(Ast_Node *expr, void *ctx) {
  struct ctx_match *ctx_match = (struct ctx_match *)ctx;
  Ast_Node *pattern = ctx_match->pattern;
  Ast_Node *replacement = ctx_match->replacement;

  Ast_Iter *it_expr = ast_iter_create(expr, PRE);
  Ast_Iter *it_patt = ast_iter_create(pattern, PRE);
  ast_begin(it_expr);
  ast_begin(it_patt);
  while (1) {
    if (it_expr->dir != it_patt->dir) {
      break;
    }
    if (patt_match(it_patt->head, it_expr->head, NULL)) {
      break;
    }
    ast_traverse(it_expr);
    ast_traverse(it_patt);
  }
  free(it_expr);
  free(it_patt);

  Ast_Node *copy = ast_copy(replacement);
  return;
}
