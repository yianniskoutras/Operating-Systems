#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MAX_SIZE 100


// I want to define a queue struct (den exw valei controls kakws gia to pote einai gemati/adeia klp, an exw xrono that to kanw me malloc opws prepei, gia tin wra douleyei)

typedef struct {
    int data[MAX_SIZE];
    int front;
    int rear;
} Queue;

void initQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
}

void enqueue(Queue *q, int value) {
    q->rear++;
    q->data[q->rear] = value;
    if (q->front == -1) {
        q->front = 0;
    }
}

//end of queue definition

int dequeue(Queue *q) {
    int value;
    value = q->data[q->front];
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        q->front++;
    }
    return value;
}



int main(int argc, char** argv) {

//beginning of controls; is the input ok?

    //ta orismata prepei na einai afstira eite 2 eite 3

    if(argc!=2 && argc!=3){

        perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 0;
    }

    //as doume an to n einai natural number

    char *endptr;
    long child_num = strtol(argv[1], &endptr, 10);
    int index_flag=0;


    if (errno != 0 || *endptr != '\0' || child_num <= 0) {
        perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 0;

    }

    //elegxoume an edwse round-robin/random/tipota

    if(argc==3){

        int result = strcmp(argv[2], "--random");

        if (!result) index_flag=1;

        else {

            result = strcmp(argv[2], "--round-robin");

            if(result!=0) {

                perror("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                return 0;
            }

        }

    }

    //end of controls: the user has successfully declared if he wants round-robin/random
    //we can now begin executing the program

    printf("I will create %ld child processes using %s\n", child_num, (index_flag==0) ? "round-robin" : "random");

    int NUM_CHILDREN = atoi(argv[1]);
    pid_t pid[NUM_CHILDREN];
    int fd[NUM_CHILDREN][2];
    int i;
    int fdc[2];

    // Create pipes
    for (i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    if(pipe(fdc) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create child processes
    for (i = 0; i < NUM_CHILDREN; i++) {
        pid[i] = fork();

        if (pid[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid[i] == 0) {  // Child process
            close(fd[i][1]);  // Close write end of pipe
            //printf("Child %d is waiting for data...\n", i+1);

            while (1) {
                char buffer[100];
                int bytes_read = read(fd[i][0], buffer, sizeof(buffer));

                if (bytes_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                    printf("Child %d received EOF\n", i+1);
                    break;
                } else {
                    printf("[Child %d] [%d] Child received %s!\n", i, getpid(), buffer);
                    sleep(5);
                    int val = atoi(buffer);
                    val++;
                    snprintf(buffer, sizeof(buffer), "%d", val);
                    printf("[Child %d] [%d] Child finished hard work, writing back %d\n", i, getpid(), val);

                    if(write(fdc[1], buffer, sizeof(buffer)) == -1){
                        perror("write");
                        exit(EXIT_FAILURE);
                    };
                }
            }

            close(fdc[1]);
            close(fd[i][0]);  // Close read ends of pipes
            exit(EXIT_SUCCESS);
        }
    }

    // Close unused pipe ends in parent process
    for (i = 0; i < NUM_CHILDREN; i++) {
        close(fd[i][0]);  // Close read end of pipe
    }

    // Parent process
    int current_child = 1;
    Queue q;
    initQueue(&q);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(fdc[0], &readfds);
        select(fdc[1] + 1, &readfds, NULL, NULL, NULL);
        //printf("Enter data: ");


        if (FD_ISSET(STDIN_FILENO, &readfds)) {

            char input[100];
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';  // Remove trailing newline character

            if (strcmp(input, "exit") == 0) {

                for (i = 0; i < NUM_CHILDREN; i++) {
                    close(fd[i][1]);
                }

                for (i = 0; i < NUM_CHILDREN; i++) {
                    printf("Waiting for %d\n", NUM_CHILDREN - i);
                    kill(pid[i], SIGTERM);
                }

                printf("All children terminated\n");
                return 0;

            }

            if (strcmp(input, "help") == 0) {
                printf("Type a number to send job to a child!\n");
                continue;
            }


            strcat(input, "\n"); // add the newline character only for the if condition test below

            long num = strtol(input, &endptr, 10);


            if (endptr == input || *endptr != '\n') {
                //printf("buffer is %s\n", buffer);
                printf("Type a number to send job to a child! \n");
                continue;
            }

            input[strcspn(input, "\n")] = '\0';  // Remove trailing newline character again

            if (!index_flag) {

                printf("[Parent] Assigned %s to child %d\n", input, current_child);
                enqueue(&q, current_child); // push index to queue

                int bytes_written = write(fd[current_child][1], input, strlen(input) + 1);

                if (bytes_written == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }

                current_child = (current_child + 1) % NUM_CHILDREN;
            }

                else {
                        srand(time(NULL)); // Seed the random number generator
                        int random_num = rand() % NUM_CHILDREN; // Generate a random number between 0 and NUM_CHILDREN (non-inclusive)


                        printf("[Parent] Assigned %s to child %d\n", input, random_num);
                        enqueue(&q, random_num);  // push index to queue

                        int bytes_written = write(fd[random_num][1], input, strlen(input) + 1);

                        if (bytes_written == -1) {
                        perror("write");
                        exit(EXIT_FAILURE);
                        }
                }
        }

        if (FD_ISSET(fdc[0], &readfds)) { // reading from child process

            // Close unused write end of pipe
            close(fdc[1]);

            // Read from pipe
            char buf[128];
            ssize_t bytes_read = read(fdc[0], buf, sizeof(buf));
            if (bytes_read == -1) {
                if (errno == EAGAIN) {
                    continue; // No data available, try again later
                }
                perror("read");
                exit(EXIT_FAILURE);
            }

            if (bytes_read > 0) {
                buf[bytes_read] = '\0';
                printf("[Parent] Received result from child %d --> %s\n", dequeue(&q) ,buf); //print index and remove it from queue
            }
        }
    }


    return 0;
}
