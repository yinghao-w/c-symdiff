#define SYMBOLS_DEBUG
#include "ast.h"
#include "../c-generics/fat_pointer.h"
#include "lexer.h"
#include "symbols.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define T_TYPE Token
#define T_PREFIX ast
#define T_STRUCT_PREFIX Ast
#include "tree.h"

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
#define fp_peek(darray) darray[fp_length(darray) - 1]

/* ------------ *
 * AST BUILDING *
 * ------------ */

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

/* ------------------- *
 * EXTRA AST FUNCTIONS *
 * ------------------- */

static Ast_Node *ast_copy(Ast_Node *root) {
  Ast_Node *copy_root = ast_leaf(root->value);
  Ast_Iter *it1 = ast_iter_create(root, T_PRE);
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

static void recursive_print(Ast_Node *node) {
  if (ast_is_leaf(node)) {
    printf(" ");
    tok_print(node->value);
    printf(" ");
    return;
  }

  printf("(");
  if (node->lchild) {
    recursive_print(node->lchild);
  }
  printf(" ");
  tok_print(node->value);
  printf(" ");
  if (node->rchild) {
    recursive_print(node->rchild);
  }
  printf(")");
}

/* Rotate the subtree rooted at node counter-clockwise. Does not check for
 * existence of the right child. */
static void ast_rotate_ccw(Ast_Node *node) {

  Ast_Node *new_top = node->rchild;
  Ast_Node *new_l_l = node->lchild;
  Ast_Node *new_l_r = NULL;
  if (new_top->lchild) {
    new_l_r = new_top->lchild;
    ast_detach(new_l_r);
  }
  ast_detach(new_l_l);
  ast_detach(new_top);

  /* If overwrite then attach, then during the overwrite, if new_top only has a
   * rchild, it will be attached to the lchild side of node. So we ensure
   * new_top has an lchild first. Function only used to with commutative
   * operator nodes, so non-issue, but wary for potential reuse. */
  Ast_Node *new_lchild = ast_join(node->value, new_l_l, new_l_r);
  ast_attach(new_lchild, new_top);
  ast_overwrite(node, new_top);
}

static void ast_reflect(Ast_Node *node) {
	Ast_Node *new_lchild = node->rchild;
	Ast_Node *new_rchild = node->lchild;
	if (new_lchild) {
		ast_detach(new_lchild);
	}
	if (new_rchild) {
		ast_detach(new_rchild);
	}
	ast_attach(new_lchild, node);
	ast_attach(new_rchild, node);
}

static void ast_swap(Ast_Node *node1, Ast_Node *node2) {
  Ast_Node *parent1 = node1->parent;
  Ast_Node *parent2 = node2->parent;
  ast_detach(node1);
  ast_detach(node2);
  /* Flipped attach order to detach ensures that nodes are swapped when they
   * share a parent. */
  ast_attach(node2, parent1);
  ast_attach(node1, parent2);
}

/* -------------------- *
 * EXPRESSION STRUCTURE *
 * -------------------- */

struct Expression {
  Ast_Node *dummy_parent;
};

Expression expr_create(char input[]) {
  // Expression *p = malloc(sizeof(*p));
  Ast_Node *ast_tree = shunting_yard(lexer(input));

  /* Give the AST root a dummy parent to simplify tree modification functions */
  Token token;
  token.token_type = VAR;
  token.var = '#';
  // p->dummy_parent = ast_join(token, ast_tree, NULL);

  // return *p;
  Expression expr = {ast_join(token, ast_tree, NULL)};
  return expr;
}

void expr_destroy(Expression expr) { ast_destroy(expr.dummy_parent); }

Ast_Node *get_root(Expression expr) { return expr.dummy_parent->lchild; }

int expr_is_equal(Expression expr1, Expression expr2) {
  return ast_is_equal(expr1.dummy_parent, expr2.dummy_parent, tok_is_equal);
}

Expression expr_copy(Expression expr) {
  Ast_Node *dummy_copy = ast_copy(expr.dummy_parent);
  return (Expression){dummy_copy};
}

void expr_print(Expression expr) { recursive_print(get_root(expr)); }

/* ---------------------------------------- *
 * EVALUATION AND SIMPLIFICATION TRANSFORMS *
 * ---------------------------------------- */

struct CtxAll {
  int changed;
  void *ctx_trans;
};

static void eval_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;

  if (T_IS_OPR(node)) {
    if (T_OPR(node)->arity == 1 && T_IS_SCALAR(node->lchild)) {

      Scalar evaled = (T_OPR(node)->func)(&T_SCALAR(node->lchild));
      Token t;
      t.token_type = SCALAR;
      t.scalar = evaled;
      Ast_Node *new = ast_leaf(t);
      ast_overwrite(node, new);

      ctx_all->changed = 1;

    } else if (T_OPR(node)->arity == 2 && T_IS_SCALAR(node->lchild) &&
               T_IS_SCALAR(node->rchild)) {

      Scalar arr[2] = {T_SCALAR(node->lchild), T_SCALAR(node->rchild)};
      Scalar evaled = (T_OPR(node)->func)(arr);
      Token t;
      t.token_type = SCALAR;
      t.scalar = evaled;
      Ast_Node *new = ast_leaf(t);
      ast_overwrite(node, new);

      ctx_all->changed = 1;
    }
  }
}

#define NAME_LENGTH 16

struct Simpl {
  char name[NAME_LENGTH];
  Opr *opr;
  Scalar x;
};

static void id_apply(Ast_Node *node, void *ctx) {
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

static void ann_apply(Ast_Node *node, void *ctx) {
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

/* If node and its right child are the same associative operator, rotates the
 * subtree counter-clockwise, i.e. changes a + (b + c) to (a + b) + c. */
static void assoc_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;
  Opr *opr = ctx_all->ctx_trans;

  if (T_IS_OPR(node) && T_OPR(node) == opr && T_IS_OPR(node->rchild) &&
      T_OPR(node->rchild) == opr) {
    ast_rotate_ccw(node);
    ctx_all->changed = 1;
  }
}

/* Nodes ordered lowest-to-highest scalars, variables, operators. Scalars and
 * variables are ordered as usual. Operators are first ordered by their initial
 * character, then by the ordering of their left children. Returns 1 if
 * node1 > node2, 0 otherwise. */
static int ast_cmp(Ast_Node *node1, Ast_Node *node2) {
  if (T_TYPE(node1) != T_TYPE(node2)) {
    return T_TYPE(node1) > T_TYPE(node2);

  } else {
    switch (T_TYPE(node1)) {
    case SCALAR:
      return T_SCALAR(node1) > T_SCALAR(node2);
      break;

    case VAR:
      return T_VAR(node1) > T_VAR(node2);
      break;

    case OPR:
      if (T_OPR(node1) != T_OPR(node2)) {
        return T_OPR(node1)->repr[0] > T_OPR(node2)->repr[0];
      } else {
        return ast_cmp(node1->lchild, node2->lchild);
      }
      break;
    }
  }
}

/* Exchange sort between the right child of node and the left child or left
 * child of left child. In essence a bubble sort pass. Becomes a bubble sort
 * when iteratively applied by expr_it_apply and norm_apply. */
static void order_apply(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;
  Opr *opr = ctx_all->ctx_trans;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {

    if (T_IS_OPR(node->lchild) && T_OPR(node->lchild) == opr) {

      /* if the first node is greater than the second in the described order */
      if (ast_cmp(node->lchild->rchild, node->rchild)) {
        ast_swap(node->lchild->rchild, node->rchild);

        ctx_all->changed = 1;
      }
    } else {
      if (ast_cmp(node->lchild, node->rchild)) {
        ast_swap(node->lchild, node->rchild);

        ctx_all->changed = 1;
      }
    }
  }
}

/* TODO: Implement whole bubble sort in one function. */
static void order_2_apply(Ast_Node *root, void *ctx) {
  // struct CtxAll *ctx_all = ctx;
  // Opr *opr = ctx_all->ctx_trans;
  //
  // while (1) {
  //   int swapped = 0;
  //   while (1) {
  //
  //     ;
  //   };
  // }
}

static void qwe(Ast_Node *node, void *ctx) {
  struct CtxAll *ctx_all = ctx;
  Opr *opr = ctx_all->ctx_trans;

  if (T_IS_OPR(node) && T_OPR(node) == opr) {
	  ;

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

/* Attempts to match the value of patt to the given node. If patt->value is a
 * a variable, binds it to the node and adds it to the list of bindings. */
static int patt_match(const Ast_Node *patt, Ast_Node *node, BindMap *bindings) {
  switch (T_TYPE(patt)) {

  case SCALAR:
    return T_SCALAR(patt) == T_SCALAR(node);
    break;

  case VAR:
    if (var_match(T_VAR(patt), node)) {

      /* Check if variable already bound, and if bound AST differs from node
       */
      if (bind_is_in(T_VAR(patt), bindings)) {
        return ast_is_equal(bind_get(T_VAR(patt), bindings), node,
                            tok_is_equal);
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
static int match(Ast_Node *pattern, Ast_Node *ast_expr, BindMap *bindings) {
  int matched = 1;

  Ast_Iter *it = ast_iter_create(pattern, T_PRE);
  Ast_Node *patt_node = ast_start(it);
  Ast_Node *expr_node = ast_expr;

  /* Walks the pattern tree. Uses the direction of the pattern tree iterator
   * to walk the expression tree. */
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
      /* Should not need to check for parent's existence; loop should end
       * right after it->head returns to pattern root. */
      expr_node = expr_node->parent;
      break;
    }
  }

loop_exit:
  free(it);
  return matched;
}

static void match_apply(Ast_Node *node, void *ctx) {
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

struct Simpl *simpls = NULL;

/* Normalisation rules to convert expression into more readily modified form. */
struct PatternRule *norm_rules = NULL;

/* Denomralisation rules to convert expression into more human readable form. */
struct PatternRule *denorm_rules = NULL;
struct PatternRule *diff_rules = NULL;

static struct PatternRule rule_make(const char name[], char pattern[],
                                    char replacement[]) {
  struct PatternRule rule;
  strncpy(rule.name, name, NAME_LENGTH);
  rule.pattern = expr_create(pattern);
  rule.replacement = expr_create(replacement);
  return rule;
}

static struct PatternRule rule_inverse(struct PatternRule *rule) {
  struct PatternRule inverse_rule;
  strncpy(inverse_rule.name, "i_", 2);
  strncpy(inverse_rule.name + 2, rule->name, NAME_LENGTH - 2);
  inverse_rule.name[NAME_LENGTH - 1] = '\0';
  inverse_rule.pattern = rule->replacement;
  inverse_rule.replacement = rule->pattern;
  return inverse_rule;
}

static struct Simpl simpl_make(const char name[], Opr *opr, Scalar x) {
  struct Simpl simpl;
  strncpy(simpl.name, name, NAME_LENGTH);
  simpl.opr = opr;
  simpl.x = x;
  return simpl;
}

void simpls_init(void) {
  fp_push(simpl_make("add id", opr_get("+"), 0), simpls);
  fp_push(simpl_make("mul id", opr_get("*"), 1), simpls);
  fp_push(simpl_make("mul ann", opr_get("*"), 0), simpls);
}

void norm_rules_init(void) {

  fp_push(rule_make("- to +", "f - g", "f + -1 * g"), norm_rules);
  fp_push(rule_make("/ to *", "f / g", "f * g ^ -1"), norm_rules);

  fp_push(rule_make("x+x = 2*x", "f + f", "2 * f"), norm_rules);
  fp_push(rule_make("x*x = x^2", "f * f", "f ^ 2"), norm_rules);

  fp_push(rule_make("x^1 = x", "f ^ 1", "f"), norm_rules);
  fp_push(rule_make("x^y^z = x^yz", "(f ^ g) ^ h", "f ^ (g * h)"), norm_rules);

  fp_push(rule_make("factor left", "f * g + h * g", "(f + h) * g"), norm_rules);
  fp_push(rule_make("factor right", "f * g + f * h", "f * (g + h)"),
          norm_rules);

  fp_push(rule_make("power left", "f ^ g * h ^ g", "(f h) ^ g"), norm_rules);
  fp_push(rule_make("power right", "f ^ g * f ^ h", "f ^ (g + h)"), norm_rules);
  /* TODO: Add separate transform for inverses */
  fp_push(rule_make("exp log = id", "exp log f", "f"), norm_rules);
  fp_push(rule_make("log exp = id", "log exp f", "f"), norm_rules);
}

void denorm_rules_init(void) {
  fp_push(rule_inverse(norm_rules), denorm_rules);
}

void diff_rules_init(void) {

  fp_push(rule_make("constant rule", "x'c", "0"), diff_rules);
  fp_push(rule_make("self rule", "x'x", "1"), diff_rules);
  fp_push(rule_make("sum rule", "x'(f + g)", "x'f + x'g"), diff_rules);
  fp_push(rule_make("product rule", "x'(f * g)", "(x'f * g) + (f * x'g)"),
          diff_rules);
  fp_push(rule_make("power rule", "x'(f ^ c)", "c * f ^ (c - 1) * x'f"),
          diff_rules);
  fp_push(rule_make("exp rule", "x'(exp f)", "exp f * x'f"), diff_rules);
  fp_push(rule_make("log rule", "x'(log f)", "f ^ -1 * x'f"), diff_rules);
  fp_push(rule_make("sine rule", "x'(sin f)", "cos f * x'f"), diff_rules);
  fp_push(rule_make("cosine rule", "x'(cos f)", "-1 * sin f * x'f"),
          diff_rules);
}

/* --------------------- *
 * RECURSIVE APPLICATION *
 * --------------------- */

static int expr_it_apply(Expression expr, ORDER order, void trans(Ast_Node *, void *),
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
    curr_changed |= expr_it_apply(expr, T_POST, ann_apply, simpls + 2);

    for (int i = 0; i < fp_length(norm_rules); i++) {
      curr_changed |= expr_it_apply(expr, T_POST, match_apply, norm_rules + i);
    }

    curr_changed |= expr_it_apply(expr, T_POST, eval_apply, NULL);
    curr_changed |= expr_it_apply(expr, T_POST, assoc_apply, opr_get("+"));
    curr_changed |= expr_it_apply(expr, T_POST, assoc_apply, opr_get("*"));

    curr_changed |= expr_it_apply(expr, T_POST, order_apply, opr_get("+"));
    curr_changed |= expr_it_apply(expr, T_POST, order_apply, opr_get("*"));

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
    curr_changed |= norm_apply(expr);
    changed |= curr_changed;
    if (!curr_changed) {
      break;
    }
  }
  return changed;
}
