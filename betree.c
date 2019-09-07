#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "betree.h"

struct node;

struct entry {
  long key;
  union {
    struct node * child; // interior node
    long value; // leaf node
  };
};

#define FANOUT   (32) // B
#define EPSILON  (8)  // #pivots
#define BUFFER   (FANOUT - EPSILON)
struct node {
  long type;         // 0: leaf;   >= 1: interior
  struct node * parent; // root's parent is NULL
  long nkeys;   // interior node: keys in the buffer
  long intr_npivots; // interior nodes only
  union {
    struct entry kvs[FANOUT]; // leaf nodes only
    struct { // interior nodes only
      struct entry pivots[EPSILON];
      struct entry buffer[BUFFER];
    };
  };
};

// interior nodes: es[0].key is always ignored
// es[0].child's keys < es[1].key <= es[1].child's keys

struct betree {
  long depth; // #levels of interior nodes, initially 0
  struct node * root;
};

  void
rec_print(struct node * const node, const long depth)
{
static const char * const head = "                                                                 ";
  printf("%.*s", depth * 4, head);
  if (node->type) {
    printf("buffer: ");
    for (long i = 0; i < node->nkeys; i++)
      printf("%ld%c", node->buffer[i].key, i == (node->nkeys -1) ? '\n' : ' ');
    printf("%.*spivots: ", depth*4, head);
    for (long i = 0; i < node->intr_npivots; i++)
      printf("%ld%c", node->pivots[i].key, i == (node->intr_npivots -1) ? '\n' : ' ');
    for (long i = 0; i < node->intr_npivots; i++)
      rec_print(node->pivots[i].child, depth + 1);
  } else {
    printf("leaf: ");
    for (long i = 0; i < node->nkeys; i++)
      printf("%ld%c", node->kvs[i].key, i == (node->nkeys -1) ? '\n' : ' ');
  }
}

  struct betree *
betree_create(void)
{
  struct betree * const tree = calloc(1, sizeof(*tree));
  // depth = 0;
  tree->root = calloc(1, sizeof(struct node)); // everything is 0
  return tree;
}

// search for __exact match__ in es
// return 0 to nkeys - 1 if found
// return -1 if not found
  static long
match_unordered(struct entry * const es, const long nkeys, const long key)
{
  // if(key == 497){
  //   printf("test ");
  //   for (long i = 0;i<nkeys;i++ )
  //     printf("%ld ", es[i]);
  //   printf("test1\n");
  // }
  for (long i = 0; i < nkeys; i++)
    if (es[i].key == key)
      return i;
  return -1;
}

// search for l where key <= es[l].key
// this is for match or insertion on the es array
// return 0 to nkeys
  static long
search_ordered(struct entry * const es, const long nkeys, const long key)
{
  long l = 0;
  long r = nkeys;
  // where to put key
  while (l != r) {
    const long x = (l + r) / 2; // assert(x < nkeys);
    if (es[x].key < key) {
      l = x+1;
    } else if (es[x].key >= key) {
      r = x;
    }
  }
  return l; // key <= es[l].key
}

// search for l where key >= es[l].key
// this is used to find a child at the next level
// es[0].key is never accessed as there is no pivot at [0]
// return 0 to nkeys - 1
  static long
search_pivot(struct entry * const es, const long nkeys, const long key)
{
  long l = 0;
  long r = nkeys - 1;
  // where to put key
  while (l != r) {
    const long x = (l + r + 1) / 2; // assert(x >= 0 && x < nkeys);
    if (es[x].key <= key) {
      l = x;
    } else if (es[x].key > key) {
      r = x-1;
    }
  }
  return l; // key >= es[l].key
}

  bool
betree_lookup(struct betree * const tree, const long key, long * const value_out)
{
  // implement betree lookup
  // ONLY use functions defined above
  // return true/false for found/not-found
  // TODO: your code here:
  // printf("%ld\ntest\n", key );
  // const long idx = bptree_find_leaf_node(tree->root, key);
  
  struct node * iter = tree->root;
  // rec_print(iter ,0 );
  // return false;
  while (true)
  {
    


    const long index = match_unordered(iter->buffer, iter->nkeys, key);
    if(index!=-1){
      *value_out = iter->buffer[index].value;
      return true;
    }

    // lookup does not need the path
    if (iter->type == 0){
      
      const long index = match_unordered(iter->kvs, iter->nkeys, key);
      
      if(index!=-1){
        *value_out = iter->kvs[index].value;
        // printf("%ld ", key);
        return true;
      }
      else{
        // printf("%ld\n", key );
        return false;
      }
      // return index;
      // for (long i = 0;i<iter->nkeys;i++ )
      //   printf("%ld ", iter[i].key);
      // printf("\n");

      // return iter;
    }
    const long idx = search_pivot(iter->pivots, iter->intr_npivots, key);
    // if(key==497){
    //   printf("%ld\n", iter->intr_npivots);
    //   printf("%ld\n", idx);
    // }
    // if(iter->pivots[idx].child)
    iter = iter->pivots[idx].child;
    count++;
    // usleep(15);
  };


  // const long idx = match_unordered(leaf->kvs, leaf->nkeys, key);
  // printf("test1\n");
  // if(idx == -1){
  //   // printf("test2\n");
  //   return false;
  // }
  // else
  //   return true;
  
  // if ((idx < leaf->nkeys) && (leaf->kvs[idx].key == key)) {
  //   // printf("test\n");
  //   *value_out = leaf->kvs[idx].value;
  //   return true;
  // } else {
  //   // printf("test3\n");
  //   return false;
  // }
}

// insert a key into a leaf node.
// return false if there is no room
  static bool
leaf_insert(struct node * const leaf, const long key, const long value)
{
  const long idx = search_ordered(leaf->kvs, leaf->nkeys , key);
  if ((idx < leaf->nkeys) && (leaf->kvs[idx].key == key)) // already there
    return false;

  if (leaf->nkeys < FANOUT) {
    count++;
    // usleep(15);
    // move larger elements right; memmove handles overlaps regions well
    memmove(&(leaf->kvs[idx+1]), &(leaf->kvs[idx]), sizeof(struct entry) * (leaf->nkeys - idx));
    leaf->nkeys++;
    leaf->kvs[idx].key = key;
    leaf->kvs[idx].value = value;
    return true;
  }
  // TRY to insert or update a key in the leaf node
  // this function ONLY look at the given node
  // return true/false for for success/failure
  // TODO: your code here:
  return false;
}

// try insert into an interior node's buffer
// return false if there is no room
  static bool
intr_insert(struct node * const intr, const long key, const long value)
{
  // printf("test\n");
  // TRY to insert or update a key in a interior node
  // this function ONLY look at the given node
  // return true/false for for success/failure
  // TODO: your code here:
  
    const long idx = search_ordered(intr->buffer, intr->nkeys , key);
    if ((idx < intr->nkeys) && (intr->buffer[idx].key == key)) // already there
      return false;

    if(intr->nkeys < BUFFER){
      // printf("test\n");
    // if (intr->nkeys < FANOUT) {
      // move larger elements right; memmove handles overlaps regions well
      memmove(&(intr->buffer[idx+1]), &(intr->buffer[idx]), sizeof(struct entry) * (intr->nkeys - idx));
      intr->nkeys++;
      intr->buffer[idx].key = key;
      intr->buffer[idx].value = value;
      count++;
      // usleep(15);
      return true;

    }
  return false;
}

  static bool
insert_one_kv(struct node * const node, const long key, const long value)
{
  return node->type ? intr_insert(node, key, value) : leaf_insert(node, key, value);
}

// create a right node and move something from left to right
  static struct node *
split_intr_right(struct node * const left)
{
  struct node * const right = calloc(1, sizeof(struct node));
  // safe type/depth, same parent
  right->type = left->type;
  right->parent = left->parent;
  memmove(right->pivots, &(left->pivots[EPSILON/2]), sizeof(struct entry) * (EPSILON/2));
  left->intr_npivots = EPSILON/2;
  right->intr_npivots = EPSILON/2;
  for (long i = 0; i < EPSILON/2; i++)
    right->pivots[i].child->parent = right;

  // move buffered keys
  const long key1 = right->pivots[0].key;
  long idxleft = 0;
  long idxright = 0;
  for (long i = 0; i < left->nkeys; i++) {
    if (left->buffer[i].key < key1) {
      left->buffer[idxleft++] = left->buffer[i];
    } else {
      right->buffer[idxright++] = left->buffer[i];
    }
  }
  left->nkeys = idxleft;
  right->nkeys = idxright;
  // parent will be updated after this function returned
  return right;
}


  static void
split_root_intr(struct betree * const tree)
{
  struct node * const left = tree->root;
  struct node * const newroot = calloc(1, sizeof(struct node));
  left->parent = newroot;
  struct node * const right = split_intr_right(left);
  newroot->type = left->type + 1; // higher
  newroot->intr_npivots = 2;
  newroot->pivots[0].key = left->pivots[0].key;
  newroot->pivots[0].child = left;
  newroot->pivots[1].key = right->pivots[0].key;
  newroot->pivots[1].child = right;
  tree->root = newroot;
}

  static void
insert_pivot(struct node * const parent, const long key, struct node * const child)
{
  assert(parent->type);
  assert(parent->intr_npivots < EPSILON);
  // insert a pivot/child pair at the given (parent) node
  // TODO: Your code here:
  // const long index = parent->intr_npivots;
  const long idx = search_ordered(parent->pivots, parent->intr_npivots, key);

  memmove(&(parent->pivots[idx+1]), &(parent->pivots[idx]), sizeof(struct entry) * (parent->intr_npivots - idx));
  // printf("test: %ld\n", search_ordered(parent->pivots, parent->intr_npivots, key));
  parent->intr_npivots++;

  parent->pivots[idx].key = key;
  parent->pivots[idx].child = child;
  
  child->parent = parent;
}

// recursively split parents
  static void
split_intr(struct betree * const tree, struct node * const intr)
{
  assert(intr->type > 0);
  assert(intr->intr_npivots == EPSILON);

  if (intr->parent == NULL) {
    split_root_intr(tree);
    return;
  }

  // make room at parent node
  if (intr->parent->intr_npivots == EPSILON)
    split_intr(tree, intr->parent);

  struct node * const right = split_intr_right(intr);
  insert_pivot(intr->parent, right->pivots[0].key, right);
}

// split a full leaf node. split full parents recursively
  static void
split_from_leaf(struct betree * const tree, struct node * const left)
{
  assert(left->type == 0);
  assert(left->nkeys == FANOUT);

  // take care of parents
  if (left->parent) { // has parent
    if (left->parent->intr_npivots == EPSILON)
      split_intr(tree, left->parent);
  } else { // leaf is root
    struct node * const newroot = calloc(1, sizeof(struct node));
    newroot->type = left->type + 1;
    newroot->intr_npivots = 1;
    newroot->pivots[0].key = LONG_MIN; // actually unused
    newroot->pivots[0].child = left;
    tree->root = newroot;
    left->parent = newroot;
  }
  struct node * const right = calloc(1, sizeof(struct node));
  right->type = left->type;
  right->parent = left->parent;
  memmove(right->kvs, left->kvs + (FANOUT/2), sizeof(struct entry) * (FANOUT/2));
  left->nkeys = FANOUT/2;
  right->nkeys = FANOUT/2;
  insert_pivot(left->parent, right->kvs[0].key, right);

}

// flush for interior node (recursion)
  static void
flush_buffer(struct betree * const tree, struct node * const node)
{
  assert(node->type != 0);
  assert(node->nkeys == BUFFER);
  while (node->nkeys) { // nkeys may be changed in recursive calls to flush/split
    const long i = node->nkeys - 1;
    const long cidx = search_pivot(node->pivots, node->intr_npivots, node->buffer[i].key);
    struct node * const child = node->pivots[cidx].child;
    if (insert_one_kv(child, node->buffer[i].key, node->buffer[i].value)) {
      
      node->nkeys--;
    } else if (child->type) { // full buffer
      flush_buffer(tree, child);
    } else { // full leaf
      split_from_leaf(tree, child);
    }
  }
}

// there could be duplicates across the levels
// we don't know at all if it's a update or insertion
  void
betree_insert(struct betree * const tree, const long key, const long value)
{
  while (insert_one_kv(tree->root, key, value) == false) { // root may change
    // count++;
    // printf("%d\n", count);
    // printf("%ld\n",tree->root->type );
    if (tree->root->type) {
      flush_buffer(tree, tree->root);
    } else {
      split_from_leaf(tree, tree->root);
    }
    // printf("////=======BEGIN\n");
    // rec_print(tree->root, 0);
    // printf("----=======END\n");
  }
}

  static void
betree_destroy_rec(struct node * const node)
{
  // recursively free all betree nodes
  // implement this function so valgrind does not complain of memory leaks at exit
  // TODO: Your code here:
  if (node->type) { // internal
    for (long i = 0; i < node->intr_npivots; i++)
      betree_destroy_rec(node->pivots[i].child);
  }
  free(node);
}

  void
betree_destroy(struct betree * const tree)
{
  betree_destroy_rec(tree->root);
  free(tree);
}
