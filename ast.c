#include "ast.h"
#include "../c-generics/fat_pointer.h"
#include "lexer.h"
#include "symbols.h"
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
  while (fp_length(oprs) > 0) {
    assert(fp_peek(oprs).opr->repr[0] != '(');
    build(out, oprs);
  }
  fp_destroy(oprs);
  Ast_Node *root = fp_pop(out);
  fp_destroy(out);
  return root;
}
Ast_Node *ast_create(char expr[]) { return shunting_yard(lexer(expr)); }

static int tok_cmp(Token t1, Token t2) {
  if (t1.token_type != t2.token_type) {
    return 0;
  } else {
    switch (t1.token_type) {
    case SCALAR:
      return t1.scalar - t2.scalar < 0.01;
      break;
    case VAR:
      return t1.var == t2.var;
      break;
    case OPR:
      return t1.opr == t2.opr;
      break;
    }
  }
}

int ast_expr_is_equal(Ast_Node *expr1, Ast_Node *expr2) {
  return ast_is_equal(expr1, expr2, tok_cmp);
}

Ast_Node *ast_copy(Ast_Node *expr) {
  Ast_Node *copy_root = ast_leaf(expr->value);
  Ast_Iter *it1 = ast_iter_create(expr, PRE);
  Ast_Iter *it2 = ast_iter_create(copy_root, PRE);
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
  return copy_root;
}
