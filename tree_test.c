#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define T_TYPE int
#define T_PREFIX i
#define T_STRUCT_PREFIX I
#define T_DEBUG
#include "tree.h"

#define ASSERT_STRUCT(cond, struct_val, printer_fn)                            \
  do {                                                                         \
                                                                               \
    if (!(cond)) {                                                             \
      printf("Assertion failed: %s\n", #cond);                                 \
      printf("In file %s, function %s, line %d\n", __FILE__, __func__,         \
             __LINE__);                                                        \
      printf("Struct state:\n");                                               \
      printer_fn(struct_val);                                                  \
      abort();                                                                 \
    }                                                                          \
  } while (0)

void int_print(int x) { printf("%d", x); }
void tree_print(I_Node *root) { i_DEBUG_PRINT(root, &int_print); }

void iter_print(I_Iter *it) {
  printf("Iterator: \n");
  printf("R:  ");
  int_print(it->root->value);
  if (it->tail && it->head) {
    printf("C:  ");
    int_print(it->tail->value);
    printf("N:  ");
    int_print(it->head->value);
  }
  printf("\nD:  %d", it->dir);
  printf("\n");
}

/*
ASSERT_STRUCT(, , tree_print);
  printf("test_ passed\n");
*/

void test_leaf(void) {
  I_Node *node = i_leaf(12);
  ASSERT_STRUCT(node->value == 12, node, tree_print);
  ASSERT_STRUCT(!node->parent, node, tree_print);
  ASSERT_STRUCT(!node->lchild, node, tree_print);
  ASSERT_STRUCT(!node->lchild, node, tree_print);
  free(node);
  printf("%s passed\n", __func__);
}
void test_join(void) {
  I_Node *node1 = i_leaf(21);
  I_Node *node2 = i_leaf(-8);
  I_Node *node3 = i_join(0, node1, node2);
  ASSERT_STRUCT(node3->value == 0, node3, tree_print);
  ASSERT_STRUCT(!node3->parent, node3, tree_print);
  ASSERT_STRUCT(node3->lchild == node1, node3, tree_print);
  ASSERT_STRUCT(node3->rchild == node2, node3, tree_print);
  ASSERT_STRUCT(node1->parent == node3, node1, tree_print);
  ASSERT_STRUCT(node2->parent == node3, node2, tree_print);

  I_Node *node4 = i_join(-5, node3, NULL);
  ASSERT_STRUCT(node4->lchild == node3, node4, tree_print);
  ASSERT_STRUCT(!node4->rchild, node4, tree_print);
  free(node1);
  free(node2);
  free(node3);
  free(node4);
  printf("%s passed\n", __func__);
}

#define TEST_TREE_SETUP()                                                      \
  I_Node *node1 = i_leaf(1);                                                   \
  I_Node *node2 = i_leaf(2);                                                   \
  I_Node *node3 = i_join(3, node2, NULL);                                      \
  I_Node *node4 = i_join(4, node1, node3);                                     \
  I_Node *node5 = i_leaf(5);                                                   \
  I_Node *node6 = i_join(6, NULL, node5);                                      \
  I_Node *node7 = i_join(7, node4, node6);

#define TEST_TREE_TEARDOWN()                                                   \
  free(node1);                                                                 \
  free(node2);                                                                 \
  free(node3);                                                                 \
  free(node4);                                                                 \
  free(node5);                                                                 \
  free(node6);                                                                 \
  free(node7);

void test_height(void) {
  TEST_TREE_SETUP();
  ASSERT_STRUCT(i_height(node7) == 3, node7, tree_print);
  I_Node *test_leaf = i_leaf(19);
  ASSERT_STRUCT(i_height(test_leaf) == 0, test_leaf, tree_print);
  free(test_leaf);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_is_leaf(void) {
  TEST_TREE_SETUP();
  ASSERT_STRUCT(!i_is_leaf(node7), node7, tree_print);
  ASSERT_STRUCT(i_is_leaf(node1), node1, tree_print);
  ASSERT_STRUCT(!i_is_leaf(node6), node6, tree_print);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_is_root(void) {
  TEST_TREE_SETUP();
  ASSERT_STRUCT(i_is_root(node7), node7, tree_print);
  ASSERT_STRUCT(!i_is_root(node4), node4, tree_print);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_num_children(void) {
  TEST_TREE_SETUP();
  ASSERT_STRUCT(i_num_children(node7) == 2, node7, tree_print);
  ASSERT_STRUCT(i_num_children(node5) == 0, node5, tree_print);
  ASSERT_STRUCT(i_num_children(node3) == 1, node5, tree_print);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_detach(void) {
  TEST_TREE_SETUP();
  i_detach(node4);
  ASSERT_STRUCT(!node4->parent, node4, tree_print);
  ASSERT_STRUCT(!node7->lchild, node7, tree_print);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_iter_create() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, POST);
  ASSERT_STRUCT(it->root == node7, it, iter_print);
  free(it);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_begin() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, POST);
  I_Node *first = i_begin_all(it);
  ASSERT_STRUCT(first == node1, it, iter_print);
  ASSERT_STRUCT(it->tail == node1, it, iter_print);
  first = i_begin_all(it);
  ASSERT_STRUCT(first == node1, it, iter_print);
  free(it);

  it = i_iter_create(node6, POST);
  ASSERT_STRUCT(i_begin_all(it) == node5, it, iter_print);
  free(it);

  I_Node *leaf = i_leaf(11);
  it = i_iter_create(leaf, POST);
  ASSERT_STRUCT(i_begin_all(it) == leaf, it, iter_print);
  free(it);
  free(leaf);

  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_next() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, POST);
  i_begin_all(it);
  ASSERT_STRUCT(i_next_all(it) == node2, it, iter_print);
  ASSERT_STRUCT(it->tail == node2, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == node3, it, iter_print);
  ASSERT_STRUCT(it->tail == node3, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == node4, it, iter_print);
  ASSERT_STRUCT(it->tail == node4, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == node5, it, iter_print);
  ASSERT_STRUCT(it->tail == node5, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == node6, it, iter_print);
  ASSERT_STRUCT(it->tail == node6, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == node7, it, iter_print);
  ASSERT_STRUCT(it->tail == node7, it, iter_print);

  ASSERT_STRUCT(i_next_all(it) == NULL, it, iter_print);
  ASSERT_STRUCT(it->tail == NULL, it, iter_print);

  free(it);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_iter() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, POST);
  I_Node *node;
  int i;
  for (node = i_begin_all(it), i = 1; !i_end_all(it); node = i_next_all(it), i++) {
    ASSERT_STRUCT(node->value == i, it, iter_print);
  }
  free(it);
  it = i_iter_create(node5, POST);
  for (node = i_begin_all(it), i = 5; !i_end_all(it); node = i_next_all(it), i++) {
    ASSERT_STRUCT(node->value == 5, it, iter_print);
  }
  assert(i == 6);
  free(it);

  it = i_iter_create(node7, POST);
  i = 0;
  for (node = i_begin_all(it); !i_end_all(it); node = i_next_all(it)) {
    I_Iter *it2 = i_iter_create(node, POST);
    for (i_begin_all(it2); !i_end_all(it2); i_next_all(it2)) {
      i++;
    }
    free(it2);
  }
  assert(i == 18);
  free(it);

  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void node_value_get(I_Node *node, void *a) {
  int *arr = (int *)a;
  arr[node->value - 1] = node->value;
}

void test_iter_apply() {
  TEST_TREE_SETUP();
  int arr[7];
  i_iter_apply(node7, POST, node_value_get, arr);
  for (int i = 0; i < 7; i++) {
    assert(arr[i] == i + 1);
  }
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_destroy() {
  TEST_TREE_SETUP();
  i_destroy(node7);
  printf("%s check for memory leaks with valgrind\n", __func__);
}

int int_is_equal(int n, int m) { return n == m; }

void test_is_equal() {
  TEST_TREE_SETUP();
  I_Node *tree1 = i_leaf(1);
  I_Node *tree2 = i_leaf(2);
  ASSERT_STRUCT(i_is_equal(tree1, tree1, int_is_equal), tree1, tree_print);
  ASSERT_STRUCT(!i_is_equal(tree2, tree1, int_is_equal), tree2, tree_print);
  I_Node *tree3 = i_join(3, tree2, NULL);
  ASSERT_STRUCT(i_is_equal(tree3, node3, int_is_equal), tree3, tree_print);
  ASSERT_STRUCT(i_is_equal(node3, tree3, int_is_equal), node3, tree_print);
  free(tree1);
  free(tree2);
  free(tree3);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}





//----------------------------------------------------------------
//------------------------------------------------------------------
//-----------------------------------------------------------------


void test_begin_pre() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, PRE);
  I_Node *first = i_begin(it);
  ASSERT_STRUCT(first == node1, it, iter_print);
  ASSERT_STRUCT(it->tail == node1, it, iter_print);
  first = i_begin(it);
  ASSERT_STRUCT(first == node1, it, iter_print);
  free(it);

  it = i_iter_create(node6, PRE);
  ASSERT_STRUCT(i_begin(it) == node5, it, iter_print);
  free(it);

  I_Node *leaf = i_leaf(11);
  it = i_iter_create(leaf, PRE);
  ASSERT_STRUCT(i_begin(it) == leaf, it, iter_print);
  free(it);
  free(leaf);

  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_next_pre() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, PRE);

  i_begin_all(it);
  ASSERT_STRUCT(i_next_all(it) == node4, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == node1, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == node3, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == node2, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == node6, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == node5, it, iter_print);
  ASSERT_STRUCT(i_next_all(it) == NULL, it, iter_print);

  free(it);
  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}

void test_iter_pre() {
  TEST_TREE_SETUP();
  I_Iter *it = i_iter_create(node7, PRE);
  I_Node *node;
  int a[] = {7, 4, 1, 3, 2, 6, 5};
  int i;
  for (node = i_begin_all(it), i = 0; !i_end_all(it); node = i_next_all(it), i++) {
    ASSERT_STRUCT(node->value == a[i], it, iter_print);
  }
  free(it);
  it = i_iter_create(node5, PRE);
  for (node = i_begin_all(it), i = 5; !i_end_all(it); node = i_next_all(it), i++) {
    ASSERT_STRUCT(node->value == 5, it, iter_print);
  }
  assert(i == 6);
  free(it);

  it = i_iter_create(node7, PRE);
  i = 0;
  for (node = i_begin_all(it); !i_end_all(it); node = i_next_all(it)) {
    I_Iter *it2 = i_iter_create(node, PRE);
    for (i_begin_all(it2); !i_end_all(it2); i_next_all(it2)) {
      i++;
    }
    free(it2);
  }
  assert(i == 18);
  free(it);

  TEST_TREE_TEARDOWN();
  printf("%s passed\n", __func__);
}











void run_tests(void) {
  printf("%s\n\n", __FILE__);
  test_leaf();
  test_join();
  test_height();
  test_is_leaf();
  test_is_root();
  test_num_children();
  test_detach();
  test_iter_create();
  test_begin();
  test_next();
  test_iter();
  test_iter_apply();
  test_destroy();
  test_is_equal();
  test_next_pre();
  test_begin_pre();
  test_iter_pre();
}

int main(void) {
  run_tests();
  return 0;
}
