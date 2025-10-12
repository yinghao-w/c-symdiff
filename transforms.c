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

struct CtxAll {
  int changed;
  void *ctx_trans;
};

static void ast_overwrite(Ast_Node *old, Ast_Node *new) {
  if (new->parent) {
    ast_detach(new);
  }

  Ast_Node *temp;

  if ((temp = old->lchild)) {
    ast_detach(temp);
    ast_destroy(temp);
  }
  if ((temp = old->rchild)) {
    ast_detach(temp);
    ast_destroy(temp);
  }

  old->value = new->value;

  if ((temp = new->lchild)) {
    ast_detach(temp);
    ast_attach(temp, old);
  }
  if ((temp = new->rchild)) {
    ast_detach(temp);
    ast_attach(temp, old);
  }

  ast_destroy(new);
}

void eval_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;

  if (T_IS_OPR(node)) {
    if (T_OPR(node)->arity == 1 && T_IS_SCALAR(node->lchild)) {

      // T_TYPE(node) = SCALAR;
      // T_SCALAR(node) = (T_OPR(node)->func)(&T_SCALAR(node->lchild));
      // ast_destroy(node->lchild);
      // node->lchild = NULL;

      Scalar evaled = (T_OPR(node)->func)(&T_SCALAR(node->lchild));
      Token t;
      t.token_type = SCALAR;
      t.scalar = evaled;
      Ast_Node *new = ast_leaf(t);
      ast_overwrite(node, new);

      ctx_all->changed = 1;

    } else if (T_OPR(node)->arity == 2 && T_IS_SCALAR(node->lchild) &&
               T_IS_SCALAR(node->rchild)) {
      // T_TYPE(node) = SCALAR;
      // Scalar arr[2] = {T_SCALAR(node->lchild), T_SCALAR(node->rchild)};
      // T_SCALAR(node) = (T_OPR(node)->func)(arr);
      //
      // ast_destroy(node->lchild);
      // ast_destroy(node->rchild);
      // node->lchild = NULL;
      // node->rchild = NULL;

      Scalar arr[2] = {T_SCALAR(node->lchild), T_SCALAR(node->rchild)};
      Scalar evaled = (T_OPR(node)->func)(arr);
      // ast_destroy(node->lchild);
      // ast_destroy(node->rchild);
      Token t;
      t.token_type = SCALAR;
      t.scalar = evaled;
      Ast_Node *new = ast_leaf(t);
      ast_overwrite(node, new);

      ctx_all->changed = 1;
    }
  }
}

struct Simpl {
  char name[NAME_LENGTH];
  Opr *opr;
  Scalar x;
};

void id_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;
  struct Simpl *simpl = ctx_all->ctx_trans;
  Opr *opr = simpl->opr;
  Scalar id = simpl->x;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {
    if (T_IS_SCALAR(node->lchild) && T_SCALAR(node->lchild) == id) {
      ast_overwrite(node, node->rchild);
      ctx_all->changed = 1;
    } else if (T_IS_SCALAR(node->rchild) && T_SCALAR(node->rchild) == id) {
      ast_overwrite(node, node->lchild);
      ctx_all->changed = 1;
    }
  }
}

void absorp_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;
  struct Simpl *simpl = ctx_all->ctx_trans;
  Opr *opr = simpl->opr;
  Scalar ann = simpl->x;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {
    if (T_IS_SCALAR(node->lchild) && T_SCALAR(node->lchild) == ann) {
      ast_overwrite(node, node->lchild);
      ctx_all->changed = 1;
    } else if (T_IS_SCALAR(node->rchild) && T_SCALAR(node->rchild) == ann) {
      ast_overwrite(node, node->rchild);
      ctx_all->changed = 1;
    }
  }
}

/* --------------------------- *
 * PATTERN MATCHING TRANSFORMS *
 * --------------------------- */

struct PatternRule {
  char name[NAME_LENGTH];
  Expression pattern;
  Expression replacement;
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

  Ast_Iter *it = ast_iter_create(pattern, T_PRE);
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
  struct CtxAll *ctx_all = ctx;
  struct PatternRule *rule = ctx_all->ctx_trans;
  Ast_Node *pattern = get_root(rule->pattern);

  BindMap *bindings = bind_create(1);

  if (match(pattern, node, bindings)) {

    Ast_Node *replacement = ast_copy(get_root(rule->replacement));

    Ast_Iter *it = ast_iter_create(replacement, T_POST);
    for (Ast_Node *repl_node = ast_begin(it); !ast_end(it);
         repl_node = ast_next(it)) {
      if (T_IS_VAR(repl_node) && bind_is_in(T_VAR(repl_node), bindings)) {
        Ast_Node *bound_node = ast_copy(bind_get(T_VAR(repl_node), bindings));
        ast_overwrite(repl_node, bound_node);
      }
    }
    ast_overwrite(node, replacement);
    ctx_all->changed = 1;
  }
  // destroy bindings in side TODO:
  bind_destroy(bindings);
}

/* ------------------------ *
 * TRANSFORM INITIALISATION *
 * ------------------------ */

#include "../c-generics/fat_pointer.h"

struct Simpl *simpls = NULL;
struct PatternRule *rules = NULL;
struct PatternRule *diff_rules = NULL;

static struct PatternRule rule_make(const char name[], char pattern[],
                                    char replacement[]) {
  struct PatternRule rule;
  strncpy(rule.name, name, NAME_LENGTH);
  rule.pattern = expr_create(pattern);
  rule.replacement = expr_create(replacement);
  return rule;
}

static struct Simpl simpl_make(const char name[], Opr *opr, Scalar x) {
  struct Simpl simpl;
  strncpy(simpl.name, name, NAME_LENGTH);
  simpl.opr = opr;
  simpl.x = x;
  return simpl;
}

void simpls_init(void) {
  fp_push(simpl_make("add id", opr_get('+'), 0), simpls);

  fp_push(simpl_make("mul id", opr_get('*'), 1), simpls);
  fp_push(simpl_make("mul ann", opr_get('*'), 0), simpls);
}

void rules_init(void) {

  fp_push(rule_make("- to +", "f - g", "f + -1 * g"), rules);
  fp_push(rule_make("/ to *", "f / g", "f * g ^ -1"), rules);
  fp_push(rule_make("x+x = 2*x", "f + f", "2 * f"), rules);
  fp_push(rule_make("x^1 = x", "f ^ 1", "f"), rules);
}

void diff_rules_init(void) {

  fp_push(rule_make("constant rule", "x'c", "0"), diff_rules);
  fp_push(rule_make("self rule", "x'x", "1"), diff_rules);
  fp_push(rule_make("sum rule", "x'(f + g)", "x'f + x'g"), diff_rules);
  fp_push(rule_make("product rule", "x'(f * g)", "(x'f * g) + (f * x'g)"),
          diff_rules);
  fp_push(rule_make("power rule", "x'(f ^ c)", "c * f ^ (c - 1) * x'f"), diff_rules);
  fp_push(rule_make("exp rule", "x'(@ f)", "@ f * x'f"), diff_rules);
}

/* --------------------- *
 * RECURSIVE APPLICATION *
 * --------------------- */

/* Maybe rethink attach detach philosophy? Overwriting would be simpler on the
 * iterators. */

int expr_it_apply(Expression expr, ORDER order, void trans(Ast_Node *, void *),
                  void *ctx_trans) {
  struct CtxAll ctx = {0, ctx_trans};
  ast_iter_apply(get_root(expr), order, trans, &ctx);
  return ctx.changed;
}

#define MAX_ITERATIONS 50

int norm_apply(Expression expr) {
  int changed = 0;

  int j = 0;
  while (j++ < MAX_ITERATIONS) {
    int curr_changed = 0;
    curr_changed |= expr_it_apply(expr, T_POST, id_apply, simpls);
    curr_changed |= expr_it_apply(expr, T_POST, id_apply, simpls + 1);
    curr_changed |= expr_it_apply(expr, T_POST, absorp_apply, simpls + 2);

    curr_changed |= expr_it_apply(expr, T_POST, match_apply, rules);
    curr_changed |= expr_it_apply(expr, T_POST, match_apply, rules + 1);
    curr_changed |= expr_it_apply(expr, T_POST, match_apply, rules + 2);
    curr_changed |= expr_it_apply(expr, T_POST, match_apply, rules + 3);

    curr_changed |= expr_it_apply(expr, T_POST, eval_apply, NULL);

    changed |= curr_changed;
    if (!curr_changed) {
      break;
    }
  }
  return changed;
}

int diff_apply(Expression expr) {
  int changed = 0;

  int j = 0;
  while (j++ < MAX_ITERATIONS) {
    int curr_changed = 0;
    for (int i = 0; i < fp_length(diff_rules); i++) {
      curr_changed |= expr_it_apply(expr, T_PRE, match_apply, diff_rules + i);
    }
    changed |= curr_changed;
    if (!curr_changed) {
      break;
    }
  }
  norm_apply(expr);
  return changed;
}
