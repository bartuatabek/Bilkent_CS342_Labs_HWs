#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "hash.h"

HashTable *ht1; // space allocated inside library

typedef struct parameter Parameter;

struct parameter {
    int head;
		int tail;
    void * value;
};

void * insertThread(void * arg) {
    Parameter * parameter = (Parameter *) arg;
    int i = 0;

    for (i = parameter -> head; i < parameter -> tail; i++) {
        hash_insert(ht1, i, parameter -> value);
    }

    pthread_exit(NULL);
}

void * deleteThread(void * arg) {
    Parameter * parameter = (Parameter *) arg;

    int i = 0;

    for (i = parameter -> head; i < parameter -> tail; i++) {
        while (hash_delete(ht1, i) < 0);
    }

    pthread_exit(NULL);
}

int main(int argc, char ** args) {
		int T = atoi(args[1]);
	  int W = atoi(args[2]);
    int N = atoi(args[3]);
		int K = atoi(args[4]);

		if (argc != 5) {
	    printf("ERROR: Invalid program arguments.\n");
			printf("Use the following syntax to run the program.\n");
			printf("./test T W N K\n");
	    exit(EXIT_FAILURE);
	  }

    ht1 = hash_init(N, K);

    int nthreads = T;
    int iret;

    Parameter parameter[nthreads];
    pthread_t threads[nthreads];
		pthread_attr_t attr[nthreads];

    //thread parameters
    for (int i = 0; i < (nthreads - 1); i++) {
        parameter[i].value = malloc(sizeof(int));
        *((int*)(parameter[i].value)) = i;
        parameter[i].head = i * W + 1;
        parameter[i].tail = (i + 1) * W + 1;
    }

    for (int i = (nthreads - 1); i < nthreads; i++) {
        parameter[i].head = (i - 1) * W + 1;
        parameter[i].tail = (i) * W + 1;
    }

    for (int i = 0; i < (nthreads - 1); i++) {
				pthread_attr_init(&attr[i]);
        iret = pthread_create(&threads[i], &attr[i], insertThread, &parameter[i]);
        if (iret) {
            fprintf(stderr, "Error - pthread_create() return code: %d\n", iret);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = (nthreads - 1); i < nthreads; i++) {
				pthread_attr_init(&attr[i]);
        iret = pthread_create(&threads[i], &attr[i], deleteThread, &parameter[i]);
        if (iret) {
            fprintf(stderr, "Error - pthread_create() return code: %d\n", iret);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    hash_destroy(ht1);
		printf("Test completed.\n");
		return 0;
}
