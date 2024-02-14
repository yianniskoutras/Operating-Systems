#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    // Check for the correct input syntax
    if (argc != 2) {
        printf("Usage: ./a.out filename\n");
        return 1;
    }

    // Print manual
    int flag = strncmp(argv[1], "--help", 6);
    if (!flag) {
        printf("Usage: ./a.out filename\n");
        return 0;
    }

    // Get filename from input
    char *filename = argv[1];

    // Check if file already exists
    struct stat st;
    int file_info = stat(filename, &st);
    if (file_info == 0 && S_ISREG(st.st_mode)) {
        printf("Error: %s already exists\n", filename);
        return 1;
    }

    // Open file in write mode or create it if it does not exist
    int fp = open(filename, O_WRONLY | O_CREAT, 0666);
    if (fp == -1) {
        perror("Error opening file\n");
        return 1;
    }

    // Keep the parent PID to use later
    pid_t grandparent_pid = getppid();

    // Create child process
    pid_t child = fork();
    if (child == -1) {
        printf("Error: fork failed\n");
        return 1;
    }

    if (child == 0) {
        // Child process writes to the file
        pid_t child_pid = getpid();
        pid_t parent_pid = getppid();
        char message[50];
        sprintf(message, "[CHILD] getpid()= %d, getppid()= %d\n", child_pid, parent_pid);
        write(fp, message, strlen(message));
        exit(0);
    } else {
        // Parent process waits for child to finish
        wait(NULL);

        // Parent process writes to the file
        pid_t parent_pid = getpid();
        char message[50];
        sprintf(message, "[PARENT] getpid()= %d, getppid()= %d\n", parent_pid, grandparent_pid);
        write(fp, message, strlen(message));
    }

    // Close the file
    close(fp);

    return 0;
}
