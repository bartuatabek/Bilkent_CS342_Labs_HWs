#include "hash.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define unlock pthread_mutex_unlock
#define lock pthread_mutex_lock

pthread_mutex_t mutexes[100];
pthread_mutex_t destroy_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Simple hash function hash = key mod N */
int hash(int key, int N) {
  if (key <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  } else {
    int hash = key % N;
    return hash;
  }
}

/* Creates a hash table of N buckets protected by M locks.
* Each bucket have an associated linked list initialized to empty list.
* Returns a pointer to the hash table created if success; otherwise returns NULL.
*/
HashTable *hash_init(int N, int K) {
  int M = N/K;

  // Check whether the parameters are within the boundaries
  if (N < MIN_N || N > MAX_N) {
    printf("ERROR: N must be between 100 - 1000.\n");
    exit(0);
    return NULL;
  } else if (M < MIN_M || M > MAX_M) {
    printf("ERROR: M must be between 10 - 1000.\n");
    exit(0);
    return NULL;
  } else if (K < MIN_K || K > MAX_K) {
    printf("ERROR: K must be between 1 - 100.\n");
    exit(0);
    return NULL;
  } else if ( M > N){
    printf("ERROR: N must be greater than or equal to M.\n");
    exit(0);
    return NULL;
  } else if( N % M != 0){
    printf("ERROR: N must be a multiple of M.\n");
    exit(0);
    return NULL;
  }
  else {
    HashTable * hash_table;
    hash_table = (HashTable *) malloc(sizeof(HashTable));

    // Initialize hash table attributes
    hash_table -> n_size = N;
    hash_table -> m_buckets = M;
    hash_table -> noOfNodes = 0;
    hash_table -> table = (Node **) malloc(sizeof(Node*) * N);

    hash_table -> k_region = K;

    for (int i = 0; i < N; i++ ) {
      hash_table -> table[i] = NULL;
    }

    // init all mutex locks
    for (int i = 0; i < (hash_table -> k_region); i++) {
      pthread_mutex_init(&mutexes[i], NULL);
    }

    return hash_table;
  }
}

/* Inserts key k and the associated value v into the hash table hp.
* This function returns 0 if success; otherwise returns -1.
* If key already presents, does nothing and returns -1.
*/
int hash_insert(HashTable *hp, int k, void* v) {
  if (k <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  }

  int loc = hash(k, hp -> n_size);
  int target_lock = loc / hp -> m_buckets;

  // Acquire the lock
  lock(&mutexes[target_lock]);

  Node * ptr = hp -> table[loc];

  if (ptr == NULL) {
    Node * insertedNode;
    insertedNode = (Node*) malloc(sizeof(Node));
    insertedNode -> next = NULL;
    insertedNode -> value = *((int*)v);
    insertedNode -> key = k;
    hp -> table[loc] = insertedNode;
    hp -> noOfNodes = hp -> noOfNodes + 1;
    unlock(&mutexes[target_lock]);
    return 0;
  } else {
    while(ptr -> next != NULL) {
      // Check in each node if k is already present.
      if(ptr -> key == k) {
        printf("ERROR: Key is already present.\n");
        unlock(&mutexes[target_lock]);
        return -1;
      }
      ptr = ptr -> next;
    }

    if (ptr -> key == k) {
      printf("ERROR: Key is already present.\n");
      unlock(&mutexes[target_lock]);
      return -1;
    }

    Node * insertedNode;
    insertedNode = (Node*)malloc(sizeof(Node));
    insertedNode -> next = NULL;
    insertedNode -> value = *((int*)v);
    insertedNode -> key = k;
    ptr -> next = insertedNode;
    hp -> noOfNodes = hp -> noOfNodes + 1;
    unlock(&mutexes[target_lock]);
    return 0;
  }
}

/* Removes key k and the associated value v from the hash
* table hp. If success returns 0, otherwise returns -1.
*/
int hash_delete(HashTable *hp, int k) {
  if (k <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  }

  // The right bucket
  int hashed = hash(k, hp -> n_size);
  // Correct lock for region K
  int kth_lock = hashed / hp -> m_buckets;

  lock(&mutexes[kth_lock]);
  Node * place = hp -> table[hashed];

  if (place != NULL) {
    if (place -> key == k) {
      hp -> table[hashed] = place -> next;
      hp -> noOfNodes = hp -> noOfNodes - 1;
      free(place);
      unlock(&mutexes[kth_lock]);
      return 0;
    }
  } else {
    unlock(&mutexes[kth_lock]);
    return -1;
  }

  Node * removed;

  while (place -> next != NULL) {
    // Found the key in place's next
    if (place -> next -> key == k) {
      // Node removed is set to place -> next
      removed = place -> next;
      // Next node of place is set to next of removed
      place -> next = place -> next -> next;
      hp -> noOfNodes = hp -> noOfNodes + 1;
      // Free and delete removed
      free(removed);
      unlock(&mutexes[kth_lock]);
      return 0;
    }
    place = place -> next;
  }

  // If the key cannot be deleted
  printf("ERROR: Key: %d cannot be deleted.\n", k);
  unlock(&mutexes[kth_lock]);

  return -1;
}

/* Updates the value of key k to be v.
* If success returns 0, otherwise returns -1.
*/
int hash_update(HashTable *hp, int k, void *v) {
  if (k <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  }

  // Search for key k
  int loc = hash(k, hp -> n_size);
  int target_lock = loc / hp -> m_buckets;

  // Acquire the lock
  lock(&mutexes[target_lock]);

  // Bucket of k is found. look for k in the bucket now
  Node * ptr = hp -> table[loc];

  if (ptr == NULL) {
    printf("ERROR: The bucket is NULL.\n");
    return -1;
  } else{
    for (ptr = hp -> table[loc]; ptr != NULL; ptr = ptr -> next) {
      if (ptr -> key == k) {
        // Update the value.
        ptr->value = *((int*)v);
        unlock(&mutexes[target_lock]);
        return 0;
      }
    }

    unlock(&mutexes[target_lock]);
    return -1;
  }
}

/* The value associated with key k is retrieved into a
* pointer variable whose address is vp.
* If success returns 0, otherwise -1.
*/
int hash_get(HashTable *hp, int k, void **vp) {
  if (k <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  }

  // finding the correct index for the key (bucket)
  int index = hash(k, hp -> n_size);
  int lock_index = index / hp -> m_buckets;

  // Locking respective region
  lock(&mutexes[lock_index]);
  Node* place = hp -> table[index];

  // Iterate to find the key
  while(place) {
    // Key is found
    if (place -> key == k) {
      *((int*)(*vp)) = place -> value;
      unlock(&mutexes[lock_index]);
      return 0;
    }
    place = place -> next;
  }
  unlock(&mutexes[lock_index]);
  return -1;
}

/* Destroys the hash table.
* Frees all resources used by it.
*/
int hash_destroy(HashTable *hp) {
  lock(&destroy_mutex);
  // Declaring the node to be deleted
  Node* dest_node = (hp) -> table[0];

  // Iterating whole table and freeing all resources
  for (int i = 0; i < (hp) -> n_size; i++) {
    dest_node = (hp) -> table[i];
    while (dest_node != NULL) {
      Node* del_ptr = dest_node;
      dest_node = dest_node -> next;
      free(del_ptr);
    }
  }

  free((hp) -> table);
  free(hp);
  hp = NULL;
  unlock(&destroy_mutex);
  return 0;
}

/* Prints the contents of the hash table. */
void print_table_content(HashTable *hp) {
  Node *cur = hp->table[0];

  // Iterate all buckets and print them as key value pairs
  for (int i = 0; i < hp->n_size; i++) {
    cur = hp->table[i];
    printf("[%d]: ", i);
    while (cur != NULL) {
      printf("[%d:%d]––>", cur->key, cur->value);
      cur = cur->next;
    }
    printf("\n");
  }
}

/* Checks whether the given key is in the table or not.
* If not inserts it else increments its value by one.
*/
int hash_increment(HashTable *hp, int k){
  if (k <= 0) {
    printf("ERROR: Invalid key value.\n");
    return -1;
  }

  // finding the correct index for the key (bucket)
  int index = hash(k, hp -> n_size);
  int lock_index = index / hp -> m_buckets;

  // Locking respective region
  lock(&mutexes[lock_index]);
  Node* place = hp -> table[index];

  if (place == NULL) {
    Node * insertedNode;
    insertedNode = (Node*) malloc(sizeof(Node));
    insertedNode -> next = NULL;
    insertedNode -> value = 1;
    insertedNode -> key = k;
    hp -> table[index] = insertedNode;
    hp -> noOfNodes = hp -> noOfNodes + 1;
    unlock(&mutexes[lock_index]);
    return 0;
  } else {
    while(place -> next != NULL) {
      // Check in each node if k is already present.
      if(place -> key == k) {
        // Update the value increment by one
        place -> value = (place -> value) +1;
        unlock(&mutexes[lock_index]);
        return 0;
      }
      place = place -> next;
    }

    if (place -> key == k) {
      place -> value = (place -> value) +1;
      unlock(&mutexes[lock_index]);
      return 0;
    }

    Node * insertedNode;
    insertedNode = (Node*)malloc(sizeof(Node));
    insertedNode -> next = NULL;
    insertedNode -> value = 1;
    insertedNode -> key = k;
    place -> next = insertedNode;
    hp -> noOfNodes = hp -> noOfNodes + 1;
    unlock(&mutexes[lock_index]);
    return 0;
  }
}
