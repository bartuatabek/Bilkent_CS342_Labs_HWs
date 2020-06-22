// CS 342 Operating Systems Spring 2019 Project #1
// A simple shell program, a command line interpreter, called bilshell.
// Bartu Atabek - 21602229 - Section 3

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX 1024
#define MAX_N 4096
#define MIN_N 1

#define READ_END 0
#define WRITE_END 1

// trims the leading and trailing space from a given string
char * trim(char *str) {
    char *end;
    // skip leading whitespace
    while (isspace(*str)) {
        str = str + 1;
    }
    // remove trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end = end - 1;
    }
    // write null character
    *(end+1) = '\0';
    return str;
}

// checks whether the program has valid aguments between the boundaries
bool hasValidArguments(int n) {
    if (n >= MIN_N && n <= MAX_N)
        return true;
    else {
        printf("ERROR: N value must be between 1 and 4096.\n");
        return false;
    }
}

// determines whether the shell to be run in interactive mode or batch mode
int determineMode(int argc, int arg) {
    if (argc == 2 && hasValidArguments(arg)) {
        printf("Interactive mode initiated.\n");
        return 0;
    } else if (argc == 3 && hasValidArguments(arg)) {
        printf("Batch mode initiated.\n");
        return 1;
    } else
        return -1;
}

// function to parse the given command string for execution
void parse(char *line, char **argv) {
    while (*line != '\0') {
        while (*line == ' ' || *line == '\t' || *line == '\n') {
            *line++ = '\0';
        }
        
        *argv++ = line;
        
        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {
            line++;
        }
    }
    *argv = NULL;
}

// a function to execute the given command with arguments using execvp system call
void execute(char **argv) {
    pid_t pid;
    int status;
    
    // forking a child process from the main
    if ((pid = fork()) < 0) {
        printf("ERROR: Forking child process failed.\n");
        exit(1);
    }
    else if (pid == 0) {
        if (execvp(*argv, argv) < 0) {
            printf("ERROR: Execution failed.\n");
            exit(1);
        }
    }
    else {
        // wait for child process to finish
        while (wait(&status) != pid) {
            ;
        }
    }
}

// a function to execute the given compound commands with arguments using pipes
void executeCompoundCommand(char **argv1, char **argv2, int n) {
    pid_t pid1, pid2;
    int status1, status2;
    int pipe1[2];
    int pipe2[2];
    
    char buffer[n];
    
    int readByte;
    int readCount = 0;
    int characterCount = 0;
    
    // create two pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        printf("ERROR: Pipe failed.\n");
        exit(1);
    }
    
    // create the first child process
    if ((pid1 = fork()) < 0) {
        printf("ERROR: Forking first child process failed.\n");
        exit(1);
    } else if (pid1 == 0) {
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[1], WRITE_END);
        
        execvp(argv1[0], argv1);
        perror("exec");
        exit(1);
    } else {
        // create the second child process
        if ((pid2 = fork()) < 0) {
            printf("ERROR: Forking second child process failed.\n");
            exit(1);
        } else if (pid2 == 0) {
            close(pipe2[1]);
            close(pipe1[0]);
            close(pipe1[1]);
            dup2(pipe2[0], 0);
            
            execvp(argv2[0], argv2);
            perror("exec");
            exit(1);
        }
        // main process to read from pipe1
        // read from pipe1 N characters at a time
        close(pipe1[1]);
        close(pipe2[0]);
        
        do {
            readByte = (int) read(pipe1[0], buffer, n);
            characterCount = characterCount + readByte;
            readCount++;
            write(pipe2[1], buffer, readByte);
        } while (readByte);
        
        // Close both ends of the pipes.
        close(pipe1[0]);
        close(pipe1[1]);
        
        close(pipe2[0]);
        close(pipe2[1]);
        
        // Wait for everything to finish and exit.
        waitpid(pid1, &status1, WUNTRACED | WCONTINUED);
        waitpid(pid2, &status2, WUNTRACED | WCONTINUED);
        
        printf("character count: %d\n", characterCount);
        printf("read-call count: %d\n", readCount-1);
        
    }
}

// checks whether the command is a compound statement or not
void isCompoundCommand(char *line, int n) {
    char * argv1[64];
    char * argv2[64];
    
    char *pPosition, *strcpy, *strcpy2;
    strcpy = strdup(line);
    strcpy2 = strcpy;
    
    for (int i = 0; i < 2; i++) {
        pPosition = strsep(&strcpy,"|");
        if (i == 0) {
            parse(trim(pPosition), argv1);
        } else if (i == 1) {
            parse(trim(pPosition), argv2);
        }
    }
    argv1[sizeof(argv1)/sizeof(argv1[0]) - 1] = NULL;
    
    executeCompoundCommand(argv1, argv2, n);
}

int main(int argc, const char * argv[]) {
    char * p;
    
    if (argc < 2) {
        printf("ERROR: Invalid program arguments.\n");
        exit(1);
    }
    
    int mode = determineMode(argc, (int) strtol(argv[1], &p, 10));
    
    if (mode == 0) {
        char line[MAX];
        char * arg[64];
        
        while (1) {
            printf("Bilshell ->>> ");
            fgets(line, MAX, stdin);
            line[strcspn(line, "\n")] = 0;
            
            if (strchr(line, '|') != NULL) {
                isCompoundCommand(line, (int) strtol(argv[1], &p, 10));
                continue;
            }
            
            parse(line, arg);
            if (strcmp(arg[0], "exit") == 0) {
                exit(0);
            }
            execute(arg);
        }
        
    } else if (mode == 1) {
        FILE * fp;
        
        char * line = NULL;
        char * arg[64];
        
        size_t lenght = 0;
        ssize_t read;
        
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            printf("ERROR: no such file found.\n");
            exit(1);
        }
        
        while ((read = getline(&line, &lenght, fp)) != -1) {
            line[strcspn(line, "\n")] = 0;
            
            if (strchr(line, '|') != NULL) {
                isCompoundCommand(line, (int) strtol(argv[1], &p, 10));
                continue;
            }
            
            parse(line, arg);
            execute(arg);
        }
        
        fclose(fp);
        if (line)
            free(line);
        printf("\n");
        exit(0);
    }

    return 0;
}
