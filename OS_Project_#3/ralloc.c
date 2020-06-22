#include "ralloc.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define unlock pthread_mutex_unlock
#define lock pthread_mutex_lock
#define wait pthread_cond_wait
#define signal pthread_cond_signal

int noOfProcessess;
int noOfResourceTypes;
int handling_method;

// Data Structures for the Banker’s Algorithm & Detection Algorithm
int available[MAX_RESOURCE_TYPES];
int max[MAX_PROCESSES][MAX_RESOURCE_TYPES];
int allocation[MAX_PROCESSES][MAX_RESOURCE_TYPES];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition[MAX_PROCESSES];

/* This function will initialize the necessary structures and variables in the
* library to do resource allocation and access control. N is the number of
* processes, and M is the number of resources types. Exist is an array of M
* integers indicating the existing resource instances in the system for each
* resource type. The parameter handling method is the deadlock handling method
* to use.
*/
int ralloc_init(int p_count, int r_count, int r_exist[], int d_handling) {
  // Check whether the parameters are within the boundaries
  if (p_count <= 0 || p_count > MAX_PROCESSES) {
    printf("ERROR: N must be between 1 - %d.\n", MAX_PROCESSES);
    return -1;
  } else if (r_count <= 0 || r_count > MAX_RESOURCE_TYPES) {
    printf("ERROR: M must be between 1 - %d.\n", MAX_RESOURCE_TYPES);
    return -1;
  } else if (d_handling < 1 || d_handling > 3) {
    printf("ERROR: Invalid handling method.");
    return -1;
  } else {
    noOfProcessess = p_count;
    noOfResourceTypes = r_count;
    handling_method = d_handling;

    // init resources existing in the system
    for (int i = 0; i < noOfResourceTypes; i++) {
      available[i] = r_exist[i];
    }

    // init all thread condition variables
    for (int i = 0; i < noOfProcessess; i++) {
      pthread_cond_init(&condition[i], NULL);
    }

    // init allocated resources to 0
    for (int i = 0; i< noOfProcessess; i++) {
      for (int j=0; j< noOfResourceTypes; j++) {
        allocation[i][j] = 0;
        max[i][j] = 0;
      }
    }
    return 0;
  }
}

/* This function will be called by an application thread when started. It will
* be used to indicate the maximum resource usage of the calling thread. The
* function will record the maximum demand information for the calling thread
* inside the library structures.
*/
int ralloc_maxdemand(int pid, int r_max[]) {
  // Acquire the lock
  lock(&mutex);

  if (pid < 0 || pid > (noOfProcessess - 1)) {
    printf("ERROR: Invalid id.\n");
    unlock(&mutex);
    return -1;
  }

  // Record the maximum demand for the calling thread
  // Init need matrix to be equal to the max matrix.
  for (int j = 0; j < noOfResourceTypes; j++) {
    max[pid][j] = r_max[j];
  }
  unlock(&mutex);
  return 0;
}

// Function to find the need of each process.
void calculateNeed(int need[][noOfResourceTypes]) {
  // Calculating Need of each process
  for (int i = 0; i < noOfProcessess; i++) {
    for (int j = 0; j < noOfResourceTypes; j++) {
      // Need[i,j] = Max[i,j] – Allocation[i,j]
      need[i][j] = max[i][j] - allocation[i][j];
    }
  }
}

// Safety algorithm to check the given state is safe or not.
int checkSafety(int need[][noOfResourceTypes]) {
  // Make a copy of available resources
  int work[noOfResourceTypes];
  for (int i = 0; i < noOfResourceTypes; i++) {
    work[i] = available[i];
  }

  // Mark all processes as infinish
  bool finish[noOfProcessess];
  for (int i = 0; i < noOfProcessess; i++) {
    finish[i] = 0;
  }

  // While all processes are not finished
  // or system is not in safe state.
  int count = 0;
  while (count < noOfProcessess) {
    // Find a process which is not finish and
    // whose needs can be satisfied with current
    // work[] resources.
    for (int p = 0; p < noOfProcessess; p++) {
      // First check if a process is finished,
      // if no, go for next condition
      if (finish[p] == 0) {
        // Check if for all resources of
        // current P need is less
        // than work
        int j;
        for (j = 0; j < noOfResourceTypes; j++)
        if (need[p][j] > work[j])
        break;

        // If all needs of p were satisfied.
        if (j == noOfResourceTypes) {
          // Add the allocated resources of
          // current P to the available/work
          // resources i.e.free the resources
          for (int k = 0 ; k < noOfResourceTypes; k++)
          work[k] += allocation[p][k];

          // Mark this p as finished
          finish[p] = 1;
          p = -1;
        }
      }
    }

    // If we could not find a next process in safe
    // sequence.
    for (int i = 0; i < noOfProcessess; i++) {
      if(finish[i] == 0) {
        return -1;
      }
      return 0;
    }
  }

  // System is in safe state
  return 0;
}

/* This function is called by a thread to request resources from the library
* The function will allocate resources if it is OK to do so.
*/
int ralloc_request(int pid, int demand[]) {

  if (pid < 0 || pid > (noOfProcessess - 1)) {
    printf("ERROR: Invalid id.\n");
    return -1;
  }

  // Acquire the lock
  lock(&mutex);
  pending_loop: ;
  bool requestedAmtCheck = true;
  bool resourcesAvailable = true;

  int need[noOfProcessess][noOfResourceTypes];
  calculateNeed(need);

  for (int i = 0; i < noOfResourceTypes; i++) {
    if (demand[i] > need[pid][i]) {
      requestedAmtCheck = false;
      break;
    }
    if (demand[i] > available[i]) {
      resourcesAvailable = false;
    }
  }

  if (!requestedAmtCheck) {
    printf("ERROR: Process has exceeded its maximum claim.\n");
    unlock(&mutex);
    return -1;
  }

  if (!resourcesAvailable) {
    // Pi must wait, since resources are not available
    // printf("Resources are not available. Waiting...\n");
    wait(&condition[pid], &mutex);
    goto pending_loop;
  }

  for (int i = 0; i < noOfResourceTypes; i++) {
    available[i] -= demand[i];
    allocation[pid][i] += demand[i];
    need[pid][i] -= demand[i];
  }

  if (handling_method == 3) {
    if (checkSafety(need) == 0) {
      // The requested resources are allocated to Pi
      unlock(&mutex);
      return 0;
    } else {
      // The old resource-allocation state is restored.
      for (int i = 0; i < noOfResourceTypes; i++) {
        available[i] += demand[i];
        allocation[pid][i] -= demand[i];
        need[pid][i] += demand[i];
      }

      // Pi must wait.
      // printf("Unsafe State. Waiting...\n");
      wait(&condition[pid], &mutex);
      goto pending_loop;
    }
  }
  unlock(&mutex);
  return 0;
}

/* This function is called by a thread to release resources. The id of the
* thread releasing resources is indicated with the pid parameter.
*/
int ralloc_release(int pid, int demand[]) {
  if (pid < 0 || pid > (noOfProcessess - 1)) {
    printf("ERROR: Invalid id.\n");
    return -1;
  }

  // Acquire the lock
  lock(&mutex);

  // Check whether there are enough resources to be released for each type
  bool canBeReleased = true;
  for (int i = 0; i < noOfResourceTypes; i++) {
    if (demand[i] > allocation[pid][i]) {
      canBeReleased = false;
      break;
    }
  }

  if(!canBeReleased) {
    printf("ERROR: Can't release more than whats allocated.\n");
    unlock(&mutex);
    return -1;
  } else {
    for (int i = 0; i < noOfResourceTypes; i++) {
      allocation[pid][i] -= demand[i];
      available[i] += demand[i];
    }

    // Check if it is now possible to satisfy any pending request.
    for (int i = 0; i < noOfProcessess; i++) {
      signal(&condition[i]);
    }

    unlock(&mutex);
    return 0;
  }
}

/* This function will check if there are any deadlocked processes at the moment.
* If there are, the respective entries in the procarray will be set to 1.
* Otherwise the values are -1. The number of deadlocked processes will be
* returned as the return value. If there is no deadlock, return value will be 0.
*/
int ralloc_detection(int procarray[]) {
  if (handling_method != 2) {
    printf("ERROR: Deadlock detection is available only in 'DEADLOCK DETECTION' mode.\n");
    return -1;
  }

  // Acquire the lock
  lock(&mutex);

  int deadlocks = 0;
  int need[noOfProcessess][noOfResourceTypes];

  // Function to calculate need matrix
  calculateNeed(need);

  // Make a copy of available resources
  int work[noOfResourceTypes];
  for (int i = 0; i < noOfResourceTypes; i++) {
    work[i] = available[i];
  }

  // Mark all processes as not finished
  bool finish[noOfProcessess];
  for (int i = 0; i < noOfProcessess; i++) {
    finish[i] = 0;
  }

  // While all processes are not finished
  // or system is not in safe state.
  int count = 0;
  while (count < noOfProcessess) {
    // Find a process which is not finish and
    // whose needs can be satisfied with current
    // work[] resources.
    for (int p = 0; p < noOfProcessess; p++) {
      // First check if a process is finished,
      // if no, go for next condition
      if (finish[p] == 0) {
        // Check if for all resources of
        // current P need is less
        // than work
        int j;
        for (j = 0; j < noOfResourceTypes; j++)
          if (need[p][j] > work[j])
            break;

        // If all needs of p were satisfied.
        if (j == noOfResourceTypes) {
          // Add the allocated resources of
          // current P to the available/work
          // resources i.e.free the resources
          for (int k = 0 ; k < noOfResourceTypes; k++)
          work[k] += allocation[p][k];

          // Mark this p as finished
          count++;
          finish[p] = 1;
          p = -1;
        }
      }
    }

    // If we could not find a next process in safe
    // sequence.
    for (int i = 0; i < noOfProcessess; i++) {
      if(finish[i] == 0) {
        deadlocks++;
        procarray[i] = 1;
      } else {
        procarray[i] = -1;
      }
    }
    unlock(&mutex);
    return deadlocks;
  }

  // System is in safe
  unlock(&mutex);
  return 0;
}

/* This function will do any clean up if necessary. It will return 0 upon
* success, -1 otherwise.
*/
int ralloc_end() {
  // No clean up is necessary
  return 0;
}
