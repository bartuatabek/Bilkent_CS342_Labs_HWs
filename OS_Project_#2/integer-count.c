#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "hash.h"

HashTable *ht1; // space allocated inside library

void * insertThread(void * arg) {
  const char * file_name = arg;

  FILE* file = fopen (file_name, "r");
  int i;

  fscanf (file, "%d", &i);
  while (!feof (file)) {
    hash_increment(ht1, i);
    fscanf (file, "%d", &i);
  }

  fclose (file);
  pthread_exit(NULL);
}

void swapNodes( Node**x, Node**y) {
    Node * temp;
    temp = (Node*) malloc(sizeof(Node));
    temp -> next = NULL;
    temp -> value = (*y)->value;
    temp -> key = (*y)->key;

    (*y)->key = (*x)->key;
    (*y)->value = (*x)->value;
    (*y)->next = (*x)->next;
    (*x)->key = temp->key;
    (*x)->value = temp->value;
    (*x)->next = temp->next;

    free(temp);
    temp = NULL;
}

void bsort(Node** nodes, int size) {
  int i = 0;
  int j = 0;
  for( i = 0; i < size - 1; i++){
  	for (j = 0; j < size-i-1; j++)
           if (nodes[j] -> key > nodes[j+1] -> key)
              swapNodes(&nodes[j], &nodes[j+1]);
  }
}

int main(int argc, const char * argv[]) {
  char * p;

  int inputFileNo = (int) strtol(argv[1], &p, 10);

  if ((inputFileNo + 3) != argc) {
    printf("ERROR: Invalid program arguments.\n");
    exit(EXIT_FAILURE);
  }

  ht1  = hash_init (1000, 100);

  int nthreads = inputFileNo;
  int iret;

  pthread_t threads[nthreads];
  pthread_attr_t attr[nthreads];

  printf("Started counting integers...\n");

  for (int i = 0; i < nthreads; i++) {
    pthread_attr_init(&attr[i]);
    iret = pthread_create(&threads[i], &attr[i], insertThread, (void *) argv[i+2]);
    if (iret) {
      fprintf(stderr, "Error - pthread_create() return code: %d\n", iret);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }

  int index = 0;
  Node* cur;
  Node* results[ht1 -> noOfNodes];

  for (int i = 0; i < ht1 -> n_size; i++) {
    cur = ht1 -> table[i];
    while (cur != NULL) {
      results[index] = cur;
      index++;
      cur = cur -> next;
    }
  }

  bsort(results, (ht1->noOfNodes));

  FILE * fp;
  fp = fopen (argv[argc-1],"w");

  for (int i = 0; i < (ht1 -> noOfNodes); i++) {
    fprintf (fp, "%d: %d\n", results[i]->key, results[i]->value);
  }

  printf("Counting completed.\n");
  printf("Results outputted to %s.\n", argv[argc-1]);
  return 0;
}
