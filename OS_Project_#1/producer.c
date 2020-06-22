// CS 342 Operating Systems Spring 2019 Project #1
// A simple program that generates m random alphanumeric
// characters and prints 1 character at a time.
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
    
    static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
    int m = (int) strtol(argv[1], &p, 10);
    
    char charBlock[N];
    
    if (N > m) {
        for (int i = 0; i < m; i++) {
            charBlock[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        write(1, charBlock, m);
        printf("\n");
        return 0;
    }
    
    for (int charCount = 0; charCount < m; charCount += N) {
        for (int i = 0; i < N; i++) {
            charBlock[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        write(1, charBlock, N);
    }
    printf("\n");
    return 0;
}
