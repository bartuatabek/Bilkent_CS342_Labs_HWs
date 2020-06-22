#ifndef HASH_H
#define HASH_H

#include <pthread.h>

#define MIN_N 100
#define MAX_N 1000
#define MIN_M 10
#define MAX_M 1000
#define MIN_K 1
#define MAX_K 100

typedef struct node Node;
typedef struct hash_table HashTable;

struct node {
  int key;
  int value;
  Node * next;
};

struct hash_table {
  int n_size;
  int k_region;
  int m_buckets;
  int noOfNodes;
  Node **table;
};

int hash (int key, int N);
HashTable *hash_init (int N, int K);
int hash_insert (HashTable * hp, int k, void *v);
int hash_delete (HashTable * hp, int k);
int hash_update (HashTable * hp, int k, void *v);
int hash_get (HashTable * hp, int k, void **vp);
int hash_destroy (HashTable * hp);
int hash_increment (HashTable * hp, int k);
void print_table_content (HashTable * hp);

#endif /* HASH_H */
