// CS 342 Operating Systems Spring 2019 Project #4
// An application program that will work with files:
// create, write, read files.
// Bartu Atabek - Section 3
// Utku Görkem Ertürk - Section 3

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "pthread.h"

#define M 20000000 // amount of content in files to process
#define MAX_CASE 3 // max no of test cases
#define MAX_FILES 100 // max no of files

static const char alphanum[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz\n";

typedef struct parameter Parameter;

struct parameter {
    int name;
};

void *t_readFile(void * arg) {
  // Read the written files
  FILE *fp;
  char buffer;
  char filename[FILENAME_MAX];
  Parameter * parameter = (Parameter *) arg;

  snprintf(filename, sizeof(filename), "%d.txt", parameter -> name);
  fp = fopen (filename, "r");

  printf("Reading file: %d.txt\n", parameter -> name);
  while ((buffer = getc(fp)) != EOF) {}

  usleep(500000);
  pthread_exit(NULL);
}

void *t_createAndWrite(void * arg) {
  // Create and write some files
  FILE *fp;
  char charBlock;
  char filename[FILENAME_MAX];
  Parameter * parameter = (Parameter *) arg;

  snprintf(filename, sizeof(filename), "%d.txt", parameter -> name);
  fp = fopen (filename, "w");

  printf("Writing file: %d.txt\n", parameter -> name);
  for (int j = 0; j < M; j++) {
      charBlock = alphanum[rand() % (sizeof(alphanum) - 1)];
      fputc(charBlock, fp);
  }
  usleep(500000);
  t_readFile(arg);
  pthread_exit(NULL);
}

void createAndWrite(int n) {
  // Create and write some files
  FILE *fp;
  char charBlock;
  char filename[FILENAME_MAX];

  for (int i = 0; i < n; i++) {
    snprintf(filename, sizeof(filename), "%d.txt", i);
    fp = fopen (filename, "w");

    printf("Writing file: %d.txt\n", i);
    for (int j = 0; j < M; j++) {
        charBlock = alphanum[rand() % (sizeof(alphanum) - 1)];
        fputc(charBlock, fp);
    }
    usleep(500000);
  }
}

void readFile(int n) {
  // Read the written files
  FILE *fp;
  char buffer;
  char filename[FILENAME_MAX];

  for (int i = 0; i < n; i++) {
    snprintf(filename, sizeof(filename), "%d.txt", i);
    fp = fopen (filename, "r");

    printf("Reading file: %d.txt\n", i);
    while ((buffer = getc(fp)) != EOF) {
    }

    usleep(500000);
  }
}

void removeFiles(int n) {
  char filename[FILENAME_MAX];

  for (int i = 0; i < n; i++) {
    snprintf(filename, sizeof(filename), "%d.txt", i);
    remove(filename);
  }
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        printf("ERROR: Invalid program arguments.\n");
        printf("Use the following syntax to run the program.\n");
  			printf("./app N caseNo\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    int caseNo = atoi(argv[2]);
    int name[n];

    if (n < 1) {
      printf("ERROR: Invalid program arguments.\n");
      printf("N should be 1 < N < 100.\n");
      exit(1);
    } else if (caseNo < 1 || caseNo > MAX_CASE) {
      printf("ERROR: Invalid program arguments.\n");
      printf("caseNo should be 1 < caseNo < %d.\n", MAX_CASE);
      exit(1);
    }

    if (caseNo == 1) {
      // Test Case 1
      createAndWrite(n);
      readFile(n);
    } else if (caseNo == 2) {
      // Test Case 2
      createAndWrite(n);
      readFile(n);

      createAndWrite(n);
      readFile(n);
    } else if (caseNo == 3) {
      int nthreads = n;
      Parameter param;
      pthread_t threads[nthreads];

      // thread init
      for (int i = 0; i < nthreads; ++i) {
        param.name = i;
        pthread_create (&threads[i], NULL, t_createAndWrite, &param);
      }

      for (int i = 0; i < nthreads; i++) {
          pthread_join(threads[i], NULL);
      }
    }

    // Add more test cases here

    removeFiles(n);
    printf("Test completed.\n");
    return 0;
}
