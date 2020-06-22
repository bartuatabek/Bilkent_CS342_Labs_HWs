// CS 342 Operating Systems Spring 2019 Project #1
// A simple program that reads m characters 1
// character at a time.
// Bartu Atabek - 21602229 - Section 3

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define N 1

int main(int argc, const char * argv[]) {
    char * p;
    
    if (argc < 2) {
        printf("ERROR: Invalid program arguments.\n");
        exit(1);
    }
    
    ssize_t bytes_read = 0;
    char charBlock[N];
    int m = (int) strtol(argv[1], &p, 10);
    
    while (bytes_read < m) {
        bytes_read += read(0, charBlock, N);
    }
    
    return 0;
}
