/* A generic implementation of binary trees using the "template-instantiation"
 * or "multiple inclusion" method. It was developed for use as an abstract
 * syntax tree, so there are no sorting or searching functions. Includes an
 * iterator construct.
 *
 *
 * Use:
 *
 * Define prefix and type for the tree, and include this file:
 *
 * 		#define T_PREFIX i
 * 		#define T_TYPE int
 * 		#include "tree.h"
 *
 * Create a leaf node with a given value and no parents nor children:
 *
 * 		i_Node *node = i_leaf(1);
 *
 * Create a node with two children:
 *
 * 		i_Node *node = i_join(5, left_child, right_child);
 *
 * Free the tree at root including all children:
 *
 * 		i_destroy(root);
 *
 * 	Detach a node from its parent, setting respective parent and child pointers
 * 	to NULL:
 *
 * 		i_detach(node);
 *
 * 	Check if two trees are equal, structurally and by value using a given
 * 	T_TYPE comparator function:
 *
 * 		i_is_equal(root1, root2, cmp);
 *
 * 	Create an iterator for the tree at root with:
 *
 * 		i_Iter *it = i_iter_create(root)
 *
 * 	Use the iterator in a for loop:
 *
 * 		for (i_begin(it); !i_end(it); i_next(it)) {
 * 			func(it->curr);
 * 		}
 *
 * 	Or:
 *
 * 		for (i_Node *node = i_begin(it); !i_end(it); node = i_next(it)) {
 * 			func(node);
 * 		}
 *
 * 	Or use callbacks:
 *
 * 		i_iter_apply(root, func(node, context), context);
 *
 * 	Other functions:
 *
 * 		i_is_leaf(node);
 * 		i_is_root(node);
 * 		i_num_children(node);
 * 		i_height(root);
 *
 */

#ifndef T_PREFIX
#ifndef T_TYPE
typedef int T_DEBUG_TYPE;
#define T_TYPE T_DEBUG_TYPE
#define T_PREFIX T_DEBUG_PREFIX
#endif
#endif

#ifdef T_PREFIX
#ifdef T_TYPE

#define T_CONCAT_2(A, B) A##_##B
#define T_CONCAT(A, B) T_CONCAT_2(A, B)

#define P_Node T_CONCAT(T_PREFIX, Node)
#define P_Iter T_CONCAT(T_PREFIX, Iter)

#include <stdlib.h>

typedef struct P_Node P_Node;
struct P_Node {
  T_TYPE value;
  P_Node *parent;
  P_Node *lchild;
  P_Node *rchild;
};

static P_Node *T_CONCAT(T_PREFIX, leaf)(T_TYPE value) {
  P_Node *p = malloc(sizeof(*p));
  p->value = value;
  p->parent = NULL;
  p->lchild = NULL;
  p->rchild = NULL;
  return p;
}

static P_Node *T_CONCAT(T_PREFIX, join)(T_TYPE value, P_Node *lchild,
                                        P_Node *rchild) {
  P_Node *p = malloc(sizeof(*p));
  p->value = value;
  p->parent = NULL;
  p->lchild = lchild;
  p->rchild = rchild;
  if (lchild) {
    lchild->parent = p;
  }
  if (rchild) {
    rchild->parent = p;
  }
  return p;
}

#define t_is_leaf(node) (!node->lchild && !node->rchild)

#define t_is_root(node) (!node->parent)

#define t_num_children(node) ((node->lchild != NULL) + (node->rchild != NULL))

static int T_CONCAT(T_PREFIX, is_leaf)(P_Node *node) { return t_is_leaf(node); }

static int T_CONCAT(T_PREFIX, is_root)(P_Node *node) { return t_is_root(node); }

static int T_CONCAT(T_PREFIX, num_children)(P_Node *node) {
  return t_num_children(node);
}

/* Detach node from its parent */
static void T_CONCAT(T_PREFIX, detach)(P_Node *node) {
  if (node->parent->lchild == node) {
    node->parent->lchild = NULL;
  } else if (node->parent->rchild == node) {
    node->parent->rchild = NULL;
  }
  node->parent = NULL;
}

typedef enum { LPARENT, RPARENT, LCHILD, RCHILD } CURR_DIR;

typedef struct {
  P_Node *root;
  P_Node *curr;
  P_Node *next;
  CURR_DIR dir;
} P_Iter;

/* From the perspective of next, computes the relationship with curr. */
static CURR_DIR T_CONCAT(T_PREFIX, curr_dir)(P_Node *curr, P_Node *next) {
  if (!next) {
    return LCHILD;
  } else if (curr == next->lchild) {
    return LCHILD;
  } else if (curr == next->rchild) {
    return RCHILD;
  } else if (curr->lchild == next) {
    return LPARENT;
  } else {
    return RPARENT;
  }
}

static P_Iter *T_CONCAT(T_PREFIX, iter_create)(P_Node *root) {
  P_Iter *p = malloc(sizeof(*p));
  p->root = root;
  return p;
}

static P_Node *T_CONCAT(T_PREFIX, traverse)(P_Iter *it) {
  if (it->curr == it->root && it->curr->parent == it->next) {
    it->curr = NULL;
  } else {
    it->curr = it->next;
    switch (it->dir) {
    case LPARENT:
    case RPARENT:
      if (it->next->lchild) {
        it->next = it->next->lchild;
      } else if (it->next->rchild) {
        it->next = it->next->rchild;
      } else {
        it->next = it->next->parent;
      }
      break;
    case LCHILD:
      if (it->next->rchild) {
        it->next = it->next->rchild;
      } else {
        it->next = it->next->parent;
      }
      break;
    case RCHILD:
      it->next = it->next->parent;
      break;
    }
    it->dir = T_CONCAT(T_PREFIX, curr_dir)(it->curr, it->next);
  }
  return it->curr;
}

static P_Node *T_CONCAT(T_PREFIX, next)(P_Iter *it) {
  while (1) {
    T_CONCAT(T_PREFIX, traverse)(it);
    if (!it->curr) {
      return NULL;
    } else if (it->dir == LCHILD || it->dir == RCHILD) {
      return it->curr;
    }
  }
}

static P_Node *T_CONCAT(T_PREFIX, start)(P_Iter *it) {
  it->curr = it->root->parent;
  it->next = it->root;
  it->dir = LPARENT;
  return T_CONCAT(T_PREFIX, traverse)(it);
}

static P_Node *T_CONCAT(T_PREFIX, begin)(P_Iter *it) {
  it->curr = it->root->parent;
  it->next = it->root;
  it->dir = LPARENT;
  return T_CONCAT(T_PREFIX, next)(it);
}

static int T_CONCAT(T_PREFIX, end)(P_Iter *it) { return !it->curr; }

static void T_CONCAT(T_PREFIX, iter_apply)(P_Node *root,
                                           void (*func)(P_Node *, void *),
                                           void *ctx) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root);
  for (T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, next)(it)) {
    func(it->curr, ctx);
  }
  free(it);
}

static size_t T_CONCAT(T_PREFIX, height)(P_Node *root) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root);
  size_t height = 1;
  size_t max = 1;
  for (T_CONCAT(T_PREFIX, start)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, traverse)(it)) {
    switch (it->dir) {
    case LPARENT:
    case RPARENT:
      height++;
      break;
    case LCHILD:
    case RCHILD:
      height--;
      break;
    }
    max = max >= height ? max : height;
  }
  return max - 1;
}

static void T_CONCAT(T_PREFIX, destroy)(P_Node *root) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root);
  P_Node *node1 = NULL;
  P_Node *node2 = NULL;
  for (T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, next)(it)) {
    node2 = it->curr;
    if (node1) {
      free(node1);
    }
    node1 = node2;
  }
  free(node1);
}

/* TODO: work on */
/* v_is_equal(u, v) should return 1 if equal, 0 if not. */
static int T_CONCAT(T_PREFIX, is_equal)(P_Node *root1, P_Node *root2,
                                        int (*v_is_equal)(T_TYPE, T_TYPE)) {
  P_Iter *it1 = T_CONCAT(T_PREFIX, iter_create)(root1);
  P_Iter *it2 = T_CONCAT(T_PREFIX, iter_create)(root2);
  T_CONCAT(T_PREFIX, start)(it1);
  T_CONCAT(T_PREFIX, start)(it2);
  while (1) {
    if (!v_is_equal(it1->curr->value, it2->curr->value)) {
      return 0;
    }
    if (it1->dir != it2->dir) {
      return 0;
  }
    T_CONCAT(T_PREFIX, traverse)(it1);
    T_CONCAT(T_PREFIX, traverse)(it2);
    if (T_CONCAT(T_PREFIX, end)(it1) != T_CONCAT(T_PREFIX, end)(it2)) {
		return 0;
    } else if (T_CONCAT(T_PREFIX, end)(it1) && T_CONCAT(T_PREFIX, end)(it2)) {
      break;
    };
  }
  return 1;
}

#ifdef T_DEBUG

#include <stdio.h>

static void T_CONCAT(T_PREFIX, DEBUG_PRINT)(P_Node *root,
                                            void (*v_print)(T_TYPE)) {
  printf("Root:\n");
  v_print(root->value);
  if (root->lchild) {
    printf(" L: ");
    v_print(root->lchild->value);
  }
  if (root->rchild) {
    printf(" R: ");
    v_print(root->rchild->value);
  }
  printf("\n\n");

  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root);
  for (T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, next)(it)) {
    v_print(it->curr->value);
    if (it->curr->parent) {
      printf(" P: ");
      v_print(it->curr->parent->value);
    }
    if (it->curr->lchild) {
      printf(" L: ");
      v_print(it->curr->lchild->value);
    }
    if (it->curr->rchild) {
      printf(" R: ");
      v_print(it->curr->rchild->value);
    }
    printf("\n");
  }
}

#endif

#undef T_TYPE
#undef T_PREFIX

#undef T_CONCAT
#undef T_CONCAT_2

#undef P_Node
#undef P_Iter

#endif
#endif
