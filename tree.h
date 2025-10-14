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
 * 	Attach or detach a node to or from its parent, setting respective parent
 * and child pointers:
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
 * 		for (i_Node *node = i_begin(it); !i_end(it); node = i_next(it))
 * { func(node);
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

/* So the code in this file gets highlighted */
#ifndef T_PREFIX
#ifndef T_TYPE
typedef int T_DEBUG_TYPE;
#define T_TYPE T_DEBUG_TYPE
#define T_PREFIX T_DEBUG_PREFIX
#define T_DEBUG
#endif
#endif

#ifdef T_PREFIX
#ifdef T_TYPE

#define T_CONCAT_2(A, B) A##_##B
#define T_CONCAT(A, B) T_CONCAT_2(A, B)

/* Optionally define a separate prefix to use for the node and iterator
 * structs. If not defined, uses the usual prefix. So functions can all be
 * snake_case, and structs can be Title_Case. */
#ifndef T_STRUCT_PREFIX
#define T_STRUCT_PREFIX T_PREFIX
#endif

#define P_Node T_CONCAT(T_STRUCT_PREFIX, Node)

#include <stdlib.h>

/* ------------------------------- *
 * BASIC DEFINITIONS AND FUNCTIONS *
 * ------------------------------- */

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

/* Attach node to its parent */
static void T_CONCAT(T_PREFIX, attach)(P_Node *child, P_Node *parent) {
  if (!parent->lchild) {
    parent->lchild = child;
  } else {
    parent->rchild = child;
  }
  child->parent = parent;
}

/* ----------------------- *
 * ITERATOR IMPLEMENTATION *
 * ------------------------*/

/* TODO: Make variables which can be const, const. */

#define P_Iter T_CONCAT(T_STRUCT_PREFIX, Iter)

typedef enum { LPARENT, RPARENT, LCHILD, RCHILD } CURR_DIR;
typedef enum { T_PRE, T_POST } ORDER;

typedef struct {
  P_Node *root;
  P_Node *tail;
  P_Node *head;
  CURR_DIR dir;
  ORDER order;
} P_Iter;

/* From the perspective of head, computes the relationship with tail. */
static CURR_DIR T_CONCAT(T_PREFIX, tail_dir)(P_Node *tail, P_Node *head) {
  if (!head) {
    return LCHILD;
  } else if (tail == head->lchild) {
    return LCHILD;
  } else if (tail == head->rchild) {
    return RCHILD;
  } else if (tail->lchild == head) {
    return LPARENT;
  } else {
    return RPARENT;
  }
}

static P_Iter *T_CONCAT(T_PREFIX, iter_create)(P_Node *root, ORDER order) {
  P_Iter *p = malloc(sizeof(*p));
  p->root = root;
  p->order = order;
  return p;
}

/* Walks the tree one adjacent node at a time, depth first and left to right. */
static P_Node *T_CONCAT(T_PREFIX, traverse)(P_Iter *it) {
  if (it->tail == it->root && it->tail->parent == it->head) {
    it->tail = NULL;
  } else {
    it->tail = it->head;
    switch (it->dir) {
    case LPARENT:
    case RPARENT:
      if (it->head->lchild) {
        it->head = it->head->lchild;
      } else if (it->head->rchild) {
        it->head = it->head->rchild;
      } else {
        it->head = it->head->parent;
      }
      break;
    case LCHILD:
      if (it->head->rchild) {
        it->head = it->head->rchild;
      } else {
        it->head = it->head->parent;
      }
      break;
    case RCHILD:
      it->head = it->head->parent;
      break;
    }
    it->dir = T_CONCAT(T_PREFIX, tail_dir)(it->tail, it->head);
  }

  switch (it->order) {
  case T_PRE:
    return it->head;
    break;
  case T_POST:
    return it->tail;
    break;
  }
}

/* The function corresponding to "begin" for adjacent node walking. Identical
 * to begin in pre-order. */
static P_Node *T_CONCAT(T_PREFIX, start)(P_Iter *it) {
  it->tail = it->root->parent;
  it->head = it->root;
  it->dir = LPARENT;
  return it->head;
}

static P_Node *T_CONCAT(T_PREFIX, next)(P_Iter *it) {
  while (1) {
    T_CONCAT(T_PREFIX, traverse)(it);
    switch (it->order) {
    case T_PRE:
      if (it->tail == it->root && (it->dir == LCHILD || it->dir == RCHILD)) {
        return NULL;
      } else if (it->dir == LPARENT || it->dir == RPARENT) {
        return it->head;
      }
      break;
    case T_POST:
      if (!it->tail) {
        return NULL;
      } else if (it->dir == LCHILD || it->dir == RCHILD) {
        return it->tail;
      }
      break;
    }
  }
}

static int T_CONCAT(T_PREFIX, end)(P_Iter *it) {
  switch (it->order) {
  case T_PRE:
    return it->tail == it->root && (it->dir == LCHILD || it->dir == RCHILD);
    break;
  case T_POST:
    return !it->tail;
    break;
  }
}

static P_Node *T_CONCAT(T_PREFIX, begin)(P_Iter *it) {
  it->tail = it->root->parent;
  it->head = it->root;
  it->dir = LPARENT;
  switch (it->order) {
  case T_PRE:
    return it->head;
  case T_POST:
    return T_CONCAT(T_PREFIX, next)(it);
  }
}

/* ---------------------------- *
 * ITERATOR DEPENDENT FUNCTIONS *
 * ---------------------------- */

static void T_CONCAT(T_PREFIX, iter_apply)(P_Node *root, ORDER order,
                                           void (*func)(P_Node *, void *),
                                           void *ctx) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root, order);
  P_Node *node;
  for (node = T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       node = T_CONCAT(T_PREFIX, next)(it)) {
    func(node, ctx);
  }
  free(it);
}

static size_t T_CONCAT(T_PREFIX, height)(P_Node *root) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root, T_PRE);
  size_t height = 0;
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
  free(it);
  return max - 1;
}

static void T_CONCAT(T_PREFIX, destroy)(P_Node *root) {
  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root, T_POST);
  P_Node *node1 = NULL;
  P_Node *node2 = NULL;
  for (T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, next)(it)) {
    node2 = it->tail;
    if (node1) {
      free(node1);
    }
    node1 = node2;
  }
  free(node1);
  free(it);
}

/* v_is_equal(u, v) should return 1 if equal, 0 if not. */
static int T_CONCAT(T_PREFIX, is_equal)(P_Node *root1, P_Node *root2,
                                        int (*v_is_equal)(T_TYPE, T_TYPE)) {
  int return_val = 1;
  P_Iter *it1 = T_CONCAT(T_PREFIX, iter_create)(root1, T_PRE);
  P_Iter *it2 = T_CONCAT(T_PREFIX, iter_create)(root2, T_PRE);
  T_CONCAT(T_PREFIX, start)(it1);
  T_CONCAT(T_PREFIX, start)(it2);
  while (1) {
    if (!v_is_equal(it1->head->value, it2->head->value)) {
      return_val = 0;
      break;
    }
    if (it1->dir != it2->dir) {
      return_val = 0;
      break;
    }
    T_CONCAT(T_PREFIX, traverse)(it1);
    T_CONCAT(T_PREFIX, traverse)(it2);
    if (T_CONCAT(T_PREFIX, end)(it1) != T_CONCAT(T_PREFIX, end)(it2)) {
      return_val = 0;
      break;
    } else if (T_CONCAT(T_PREFIX, end)(it1) && T_CONCAT(T_PREFIX, end)(it2)) {
      return_val = 1;
      break;
    };
  }
  free(it1);
  free(it2);
  return return_val;
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

  P_Iter *it = T_CONCAT(T_PREFIX, iter_create)(root, T_POST);
  for (T_CONCAT(T_PREFIX, begin)(it); !T_CONCAT(T_PREFIX, end)(it);
       T_CONCAT(T_PREFIX, next)(it)) {
    v_print(it->tail->value);
    if (it->tail->parent) {
      printf(" P: ");
      v_print(it->tail->parent->value);
    }
    if (it->tail->lchild) {
      printf(" L: ");
      v_print(it->tail->lchild->value);
    }
    if (it->tail->rchild) {
      printf(" R: ");
      v_print(it->tail->rchild->value);
    }
    printf("\n");
  }
  free(it);
}

#endif

#undef T_TYPE
#undef T_PREFIX
#undef T_STRUCT_PREFIX
#undef T_DEBUG

#undef T_CONCAT
#undef T_CONCAT_2

#undef P_Node
#undef P_Iter

#endif
#endif
