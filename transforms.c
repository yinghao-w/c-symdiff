#include "ast.h"
#include "symbols.h"

#define TTYPE(node) (node->value.token_type)
#define TSCALAR(node) (node->value.scalar)
#define TVAR(node) (node->value.var)
#define TOPR(node) (node->value.opr)

#define TISSCALAR(node) (TTYPE(node) == SCALAR)
#define TISOPR(node) (TTYPE(node) == OPR)

static void ast_overwrite(Ast_Node *old, Ast_Node *new, int destroy) {
  if (old->lchild) {
    ast_detach(old->lchild);
    destroy ? ast_destroy(old->lchild), 0 : 0;
  }
  if (old->rchild) {
    ast_detach(old->rchild);
    destroy ? ast_destroy(old->rchild), 0 : 0;
  }
  old->value = new->value;
  old->lchild = new->lchild;
  old->rchild = new->rchild;
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
  if (TISOPR(expr)) {
    if (TOPR(expr)->arity == 1 && TISSCALAR(expr->lchild)) {
      TTYPE(expr) = SCALAR;
      TSCALAR(expr) = (TOPR(expr)->func)(&TSCALAR(expr->lchild));
      ast_destroy(expr->lchild);
      base->changed = 1;
    } else if (TOPR(expr)->arity == 2 && TISSCALAR(expr->lchild) &&
               TISSCALAR(expr->rchild)) {
      TTYPE(expr) = SCALAR;
      Scalar arr[2] = {TSCALAR(expr->lchild), TSCALAR(expr->rchild)};
      TSCALAR(expr) = (TOPR(expr)->func)(arr);
      ast_destroy(expr->lchild);
      ast_destroy(expr->rchild);
      base->changed = 1;
    }
  }
}

struct ctx_simpl {
  int changed;
  struct ctx_base base;
  Opr *opr;
  Scalar x;
};

void id_apply(Ast_Node *expr, void *ctx) {
  struct ctx_simpl *ctx_simpl = (struct ctx_simpl *)ctx;
  Opr *opr = ctx_simpl->opr;
  Scalar id = ctx_simpl->x;
  ctx_simpl->base.changed = 0;
  if (TISOPR(expr) && TOPR(expr) == opr) {
    if (TISSCALAR(expr->lchild) && TSCALAR(expr->lchild) == id) {
      ast_overwrite(expr, expr->rchild, 1);
      ctx_simpl->base.changed = 1;
    } else if (TISSCALAR(expr->rchild) && TSCALAR(expr->rchild) == id) {
      ast_overwrite(expr, expr->lchild, 1);
      ctx_simpl->base.changed = 1;
    }
  }
}

void absorpt_apply(Ast_Node *expr, void *ctx) {
  struct ctx_simpl *ctx_simpl = (struct ctx_simpl *)ctx;
  Opr *opr = ctx_simpl->opr;
  Scalar id = ctx_simpl->x;
  ctx_simpl->base.changed = 0;
  if (TISOPR(expr) && TOPR(expr) == opr) {
    if (TISSCALAR(expr->lchild) && TSCALAR(expr->lchild) == id) {
      ast_overwrite(expr, expr->lchild, 1);
      ctx_simpl->base.changed = 1;
    } else if (TISSCALAR(expr->rchild) && TSCALAR(expr->rchild) == id) {
      ast_overwrite(expr, expr->rchild, 1);
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
    return TISSCALAR(expr);
  default:
    return 1;
  }
}

static int patt_match(Ast_Node *patt, Ast_Node *expr, Token **table_placeholder) {
  switch (TTYPE(patt)) {
  case SCALAR:
    return TSCALAR(patt) == TSCALAR(expr);
    break;
  case VAR:
    if (patt_var_match(TVAR(patt), expr)) {
		*table_placeholder = NULL;
		// set (key, value) to (patt_var, expr)
		return 1;
	} else {
		return 0;
	}
    break;
  case OPR:
    return TOPR(patt) == TOPR(expr);
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
