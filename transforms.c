#include "ast.h"
#include "symbols.h"
#include <assert.h>
#include <string.h>

/* ------------- *
 * HELPER MACROS *
 * ------------- */

#define T_TYPE(node) (node->value.token_type)
#define T_SCALAR(node) (node->value.scalar)
#define T_VAR(node) (node->value.var)
#define T_OPR(node) (node->value.opr)

#define T_IS_SCALAR(node) (T_TYPE(node) == SCALAR)
#define T_IS_VAR(node) (T_TYPE(node) == VAR)
#define T_IS_OPR(node) (T_TYPE(node) == OPR)

#define NAME_LENGTH 16

/* ---------------------------------------- *
 * EVALUATION AND SIMPLIFICATION TRANSFORMS *
 * ---------------------------------------- */

struct Ctx {
	Expression *expr;
	int changed;
	void *trans_ctx;
};

static void ast_attach(Ast_Node *child, Ast_Node *parent) {
  assert(ast_num_children(parent) < 2);
  if (!parent->lchild) {
    parent->lchild = child;
  } else {
    parent->rchild = child;
  }
  child->parent = parent;
}

/* Replaces a new's parent with new. The wrapping of each AST root allows
 * this to be a replacement, not a overwriting. */
static void ast_merge_replace(Ast_Node *new, Expression *expr) {
  assert(new->parent);
  Ast_Node *parent = new->parent;
  ast_detach(new);

  if (ast_is_root(parent)) {
    ast_destroy(parent);
    expr->ast_tree = new;
  } else {
    Ast_Node *grandparent = parent->parent;
    ast_detach(parent);
    ast_destroy(parent);

    /* Assumes that a node will never have a rchild but no lchild. Might need to
     * change below if the assumption changes. */
    if (!grandparent->lchild) {
      grandparent->lchild = new;
    } else {
      grandparent->rchild = new;
    }
    /* TODO: add a attach function to tree.h */
    new->parent = grandparent;
  }
}

/* Replaces a node with another. Assumes the two nodes were not connected
 * beforehand. */
static void ast_replace(Ast_Node *old, Ast_Node *new, Expression *expr) {
  if (new->parent) {
    ast_detach(new);
  }

  if (ast_is_root(old)) {
    expr->ast_tree = new;
    ast_destroy(old);
  } else {
    Ast_Node *parent = old->parent;
    ast_detach(old);
    ast_destroy(old);
    ast_attach(new, parent);
  }
}

struct CtxBase {
  Expression *expr;
  int changed;
};

void eval_apply(Ast_Node *node, void *ctx) {
  struct CtxBase *base = (struct CtxBase *)ctx;

  if (T_IS_OPR(node)) {
    if (T_OPR(node)->arity == 1 && T_IS_SCALAR(node->lchild)) {
      T_TYPE(node) = SCALAR;
      T_SCALAR(node) = (T_OPR(node)->func)(&T_SCALAR(node->lchild));
      ast_destroy(node->lchild);
      node->lchild = NULL;
      base->changed = 1;

    } else if (T_OPR(node)->arity == 2 && T_IS_SCALAR(node->lchild) &&
               T_IS_SCALAR(node->rchild)) {
      T_TYPE(node) = SCALAR;
      Scalar arr[2] = {T_SCALAR(node->lchild), T_SCALAR(node->rchild)};
      T_SCALAR(node) = (T_OPR(node)->func)(arr);
      ast_destroy(node->lchild);
      ast_destroy(node->rchild);
      node->lchild = NULL;
      node->rchild = NULL;
      base->changed = 1;
    }
  }
}

struct Simpl {
	char name[NAME_LENGTH];
	Opr *opr;
	Scalar x;
};

struct CtxSimpl {
  struct CtxBase base;
  struct Simpl simpl;
};

void id_apply(Ast_Node *node, void *ctx) {
  struct CtxSimpl *ctx_simpl = (struct CtxSimpl *)ctx;
  Opr *opr = ctx_simpl->simpl.opr;
  Scalar id = ctx_simpl->simpl.x;
  Expression *expr = ctx_simpl->base.expr;
  ctx_simpl->base.changed = 0;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {
    if (T_IS_SCALAR(node->lchild) && T_SCALAR(node->lchild) == id) {
      ast_merge_replace(node->rchild, expr);
      ctx_simpl->base.changed = 1;
    } else if (T_IS_SCALAR(node->rchild) && T_SCALAR(node->rchild) == id) {
      ast_merge_replace(node->lchild, expr);
      ctx_simpl->base.changed = 1;
    }
  }
}

void absorp_apply(Ast_Node *node, void *ctx) {
  struct CtxSimpl *ctx_simpl = (struct CtxSimpl *)ctx;
  Opr *opr = ctx_simpl->simpl.opr;
  Scalar id = ctx_simpl->simpl.x;
  Expression *expr = ctx_simpl->base.expr;
  ctx_simpl->base.changed = 0;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {
    if (T_IS_SCALAR(node->lchild) && T_SCALAR(node->lchild) == id) {
      ast_merge_replace(node->lchild, expr);
      ctx_simpl->base.changed = 1;
    } else if (T_IS_SCALAR(node->rchild) && T_SCALAR(node->rchild) == id) {
      ast_merge_replace(node->rchild, expr);
      ctx_simpl->base.changed = 1;
    }
  }
}

/* --------------------------- *
 * PATTERN MATCHING TRANSFORMS *
 * --------------------------- */

struct PatternRule {
  char name[NAME_LENGTH];
  Expression *pattern;
  Expression *replacement;
};

struct CtxPattern {
  struct CtxBase base;
  struct PatternRule rule;
};

/* Instantiate a associative array to store variable-node bindings. */
#define DPX_KT Var
#define DPX_VT Ast_Node *
#define DPX_PFX bind
#define DPX_STRUCT_PFX Bind
#include "dpx.h"

static int var_match(Var x, const Ast_Node *node) {
  switch (x) {
  case 'c':
    return T_IS_SCALAR(node);
  default:
    return 1;
  }
}

/* Attempts to match the value of patt to the given node. If patt->value is a a
 * variable, binds it to the node and adds it to the list of bindings. */
static int patt_match(const Ast_Node *patt, Ast_Node *node, BindMap *bindings) {
  switch (T_TYPE(patt)) {

  case SCALAR:
    return T_SCALAR(patt) == T_SCALAR(node);
    break;

  case VAR:
    if (var_match(T_VAR(patt), node)) {

      /* Check if variable already bound, and if bound AST differs from node */
      if (bind_is_in(T_VAR(patt), bindings)) {
        return ast_is_equal(bind_get(T_VAR(patt), bindings), node, tok_cmp);
      } else {
        bind_add(T_VAR(patt), node, bindings);
        return 1;
      }

    } else {
      return 0;
    }
    break;

  case OPR:
    return T_OPR(patt) == T_OPR(node);
    break;
  }
}

/* Attempts to match the entire pattern to the expression rooted at ast_expr.
 * Adds bindings if any to bindings. */
int match(Ast_Node *pattern, Ast_Node *ast_expr, BindMap *bindings) {
  int matched = 1;

  Ast_Iter *it = ast_iter_create(pattern, PRE);
  Ast_Node *patt_node = ast_start(it);
  Ast_Node *expr_node = ast_expr;

  /* Walks the pattern tree. Uses the direction of the pattern tree iterator to
   * walk the expression tree. */
  while (!ast_end(it)) {
    if (!patt_match(patt_node, expr_node, bindings)) {
      matched = 0;
      break;
    }
    patt_node = ast_traverse(it);
    switch (it->dir) {

    case LPARENT:
      if (expr_node->lchild) {
        expr_node = expr_node->lchild;
      } else {
        matched = 0;
        goto loop_exit;
      }
      break;

    case RPARENT:
      if (expr_node->rchild) {
        expr_node = expr_node->rchild;
      } else {
        matched = 0;
        goto loop_exit;
      }
      break;

    case LCHILD:
    case RCHILD:
      /* Should not need to check for parent's existence; loop should end right
       * after it->head returns to pattern root. */
      expr_node = expr_node->parent;
      break;
    }
  }

loop_exit:
  free(it);
  return matched;
}

void match_apply(Ast_Node *node, void *ctx) {
  struct CtxPattern *ctx_match = (struct CtxPattern *)ctx;
  Ast_Node *pattern = ctx_match->rule.pattern->ast_tree;
  Expression *expr = ctx_match->base.expr;

  BindMap *bindings = bind_create(1);

  if (match(pattern, node, bindings)) {

    Ast_Node *replacement = ctx_match->rule.replacement->ast_tree;
    for (size_t i = 0; i < bind_size(bindings); i++) {
      ast_detach(bindings->data[i].value);
    }

    Ast_Iter *it = ast_iter_create(replacement, POST);
    for (Ast_Node *repl_node = ast_begin(it); !ast_end(it);
         repl_node = ast_next(it)) {
      if (T_IS_VAR(repl_node) && bind_is_in(T_VAR(repl_node), bindings)) {
        Ast_Node *bound_node = bind_get(T_VAR(repl_node), bindings);
        ast_replace(repl_node, bound_node, expr);
      }
    }
    ast_replace(node, replacement, expr);

  } else {
    ctx_match->base.changed = 0;
  }
  bind_destroy(bindings);
}

/* ------------------------ *
 * TRANSFORM INITIALISATION *
 * ------------------------ */


#define DPX_KT int
#define DPX_VT struct Simpl
#define DPX_PFX simpl 
#define DPX_STRUCT_PFX Simpl
#include "dpx.h"

SimplMap *simpls;

#define DPX_KT int
#define DPX_VT struct PatternRule
#define DPX_PFX rule
#define DPX_STRUCT_PFX Rule
#include "dpx.h"

RuleMap *rules;
RuleMap *diff_rules;

static struct PatternRule rule_make(const char name[], char pattern[],
                                    char replacement[]) {
  struct PatternRule rule;
  strncpy(rule.name, name, NAME_LENGTH);
  rule.pattern = expr_create(pattern);
  rule.replacement = expr_create(replacement);
  return rule;
}

void simpls_init(void) {
	simpls = simpl_create(1);

	simpl_add(0, (struct Simpl){"add id", opr_get('+'), 0}, simpls);
	simpl_add(1, (struct Simpl){"mul id", opr_get('*'), 1}, simpls);
	simpl_add(2, (struct Simpl){"mul ann", opr_get('*'), 0}, simpls);
}

void rules_init(void) {
  rules = rule_create(1);

  rule_add(0, rule_make("- to +", "f - g", "f + -1 * g"), rules);
  rule_add(1, rule_make("/ to *", "f / g", "f * g ^ -1"), rules);
  rule_add(2, rule_make("x+x = 2*x", "f + f", "2 * f"), rules);
}

void diff_rules_init(void) {
  diff_rules = rule_create(1);

  rule_add(0, rule_make("constant rule' = 0", "x'c", "0"), diff_rules);
  rule_add(1, rule_make("self rule' = 1", "x'x", "1"), diff_rules);
  rule_add(2, rule_make("sum rule", "x'(f + g)'", "x'f + x'g"), diff_rules);
  rule_add(3, rule_make("product rule", "x'(f * g)'", "(x'f * g) + (f * x'g)"),
           diff_rules);
  rule_add(1, rule_make("exp rule", "x'(@ f)", "@ f * x'f"), diff_rules);
}

/* --------------------- *
 * RECURSIVE APPLICATION *
 * --------------------- */

int recursive_apply(Ast_Node *node, ORDER order, void trans(Ast_Node *, void *),
                    void *ctx) {
  struct CtxBase *base = (struct CtxBase *)ctx;
  ast_iter_apply(node, order, trans, ctx);
  return base->changed;
}

int rec_apply(Expression *expr, ORDER order, void trans(Ast_Node *, void *), void *ctx_trans) {
	struct Ctx ctx = {expr, 0, ctx_trans};
	ast_iter_apply(expr->ast_tree, order, trans, &ctx);
	return ctx.changed;
}
