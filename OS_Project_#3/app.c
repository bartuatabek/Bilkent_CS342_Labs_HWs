#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "ralloc.h"

#define M 3 // number of resource types
#define N 5 // number of processes - threads

int finished[N];

typedef struct parameter Parameter;

struct parameter {
    int pid;
		int max[N][M];
    int request[M];
};

void * process(void * arg) {
  Parameter * parameter = (Parameter *) arg;
  int req[M];

  ralloc_maxdemand(parameter -> pid, parameter -> max);

  // call request and release as many times with different parameters
  // customize here
  for (int i = 0; i < 10; i++) {
    ralloc_request(parameter -> pid, parameter -> request);

    // do something with the resources
    sleep(1);

    ralloc_request(parameter -> pid, parameter -> request);

    req[0] = 4;
    req[1] = 4;
    req[2] = 4;
    ralloc_release(parameter -> pid, req);
  }
  finished[parameter -> pid] = 1;
  printf("Finished thread %d.\n", parameter -> pid);
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int success = 0;
    int deadlocks = 0; // number of deadlocked processes
    int deadlocked[N]; // array indicating deadlocked processes

    int pids[N];
    int nthreads = N;
    pthread_t threads[nthreads];
    Parameter parameter[nthreads];

    int exist[3] = {10, 10, 10}; // resources existing in the system
    int handling_method = DEADLOCK_AVOIDANCE; // deadlock handling method

    success = ralloc_init(N, M, exist, handling_method);
    if (success != 0) {
	    printf("ERROR: Library initialization failed.\n");
	    exit(EXIT_FAILURE);
	  }

    printf("Library initialized.\n");

    for (int k = 0; k < N; k++) {
      pids[k] = k;
      deadlocked[k] = -1;
    }

    // thread parameters
    // customize here
    Parameter param0 = {0, {10,10,10}, {2,2,2}};
    parameter[0] = param0;

    Parameter param1 = {1, {10,10,10}, {2,2,2}};
    parameter[1] = param1;

    Parameter param2 = {2, {10,10,10}, {2,2,2}};
    parameter[2] = param2;

    Parameter param3 = {3, {10,10,10}, {2,2,2}};
    parameter[3] = param3;

    Parameter param4 = {4, {10,10,10}, {2,2,2}};
    parameter[4] = param4;

    // thread init
    for (int i = 0; i < N; ++i) {
      pthread_create (&threads[i], NULL, process, &parameter[i]);
    }
    printf("Threads created.\n");

    while (1) {
        sleep(10); // detection period
        if (handling_method == DEADLOCK_DETECTION) {
            deadlocks = ralloc_detection(deadlocked);
            if (deadlocks > 0) {
              printf("%d processes are deadlocked.\n Processes ", deadlocks);
              for (int i = 0; i < N; i++) {
                if (deadlocked[i] == 1) {
                  printf("P%d ", i);
                }
              }
              printf("are deadlocked processes.\n");
            }
        }
        // if all treads terminated, call ralloc_end and exit
        int completed = 1;
        for (int i = 0; i < N; i++) {
          if (finished[i] != 1) {
            completed = -1;
          }
        }

        if (completed == 1) {
          ralloc_end();
          printf("Test completed.\n");
          return 0;
        }
    }
}
