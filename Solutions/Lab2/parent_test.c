#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#define MAX 10


int sig_value(char *sig_name);

pid_t child_pids[MAX];
int num_gates;
char *gatestate;


void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    printf("inside term handler\n");

    // Wait for any child process to terminate.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0 && WIFSIGNALED(status)) {

        printf("Child process %d was manually terminated.\n", pid);

        int deleted_gate;
        for (int i = 0; i < num_gates; ++i)
            if (child_pids[i] == pid)
                deleted_gate = i;

        pid_t new_pid = fork();
        time_t start = time(NULL);


        if (new_pid == -1) {
            perror("fork failed");
            exit(1);
        } else if (new_pid == 0) {
            //printf("start time in seconds is %ld\n", start);
            char start_str[20];
            sprintf(start_str, "%ld", (long) start); // convert time_t to string
            char i_str[2];
            sprintf(i_str, "%d", deleted_gate);
            char *const argv[] = {"./child", i_str, &gatestate[deleted_gate], start_str, NULL};
            int status = execv("./child", argv);
            if (status == 1) {
                perror("execv");
                exit(EXIT_FAILURE);
            }
        } else { // new_pid != 0 father running
            child_pids[deleted_gate] = new_pid;         // replace the old one.
            int parent_id = getpid();
            printf("[PARENT/PID]=%d created child %d (PID=%d) and initial state '%c'\n", parent_id, deleted_gate,
                   child_pids[deleted_gate],
                   gatestate[deleted_gate]);

        }

    }
}

void sigterm_handler(int sig) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    for (int i = 0; i < num_gates; ++i) {
        printf("Waiting for %d children to exit\n", num_gates + 1);
        kill(child_pids[i], SIGTERM);
        int status;
        waitpid(child_pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            printf("child with pid = %d terminated successfully with exitstatus 0! \n", child_pids[i]);
    }

    printf("All children exited. Terminating as well.\n");
    exit(0);
}



int main(int argc, char **argv) {

// checking for input errors


    if (argc != 2) {
        printf("Usage: ./a.out filename\n");
        return 1;
    }


    int flag = strncmp(argv[1], "--help", 6);

    if (!flag) {
        printf("Usage: ./a.out filename\n");
        return 0;
    }


    num_gates = strlen(argv[1]);
    gatestate = argv[1];


    for (int i = 0; i < num_gates; i++) {
        if (gatestate[i] != 'f' && gatestate[i] != 't') {
            printf("Usage: ./a.out filename\n");
            return 1;
        }
    }



//   input is correct, we can begin executing the desired program



//    int fd[2]; // file discriptor
    pid_t pid;
    time_t start = time(NULL);



    for (int i = 0; i < num_gates; i++) {

        pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
//            // Child process reads from pipe.
//            close(fd[1]); // close unused write end
//            dup2(fd[0], STDIN_FILENO); // Redirect standard input to pipe read end.
            //printf("start time in seconds is %ld\n", start);
            char start_str[20];
            sprintf(start_str, "%ld", (long) start); // convert time_t to string
            char i_str[2];
            sprintf(i_str, "%d", i);
            char *const argv[] = {"./child", i_str, &gatestate[i], start_str, NULL};
            int status = execv("./child", argv);
            if (status == 1) {
                perror("execv");
                exit(EXIT_FAILURE);
            }

        } else { // pid != 0 father running
//            // Parent process writes to pipe.
//            close(fd[0]); // CLOSES READ END
//            dup2(fd[1], STDOUT_FILENO); // redirect stdout to pipe end
            child_pids[i] = pid;
            int parent_id = getpid();
            printf("[PARENT/PID]=%d created child %d (PID=%d) and initial state '%c'\n", parent_id, i, child_pids[i],
                   gatestate[i]);

        }
    }



    // connect main with sigchld handler.
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    // connect main with sigchld handler.
    struct sigaction sa1;
    sa1.sa_handler = sigterm_handler;
    sa1.sa_flags = 0;
    sigemptyset(&sa1.sa_mask);
    sigaction(SIGTERM, &sa1, NULL);






    fd_set readfds;
    char buffer[1024];

    while (1) sleep(1);

    while (1) {
        // Set up the file descriptor set to monitor stdin (file descriptor 0).
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);

        // Use select() to wait for input on stdin.
        int ret = select(1, &readfds, NULL, NULL, NULL);
        if (ret == -1) {
            if (errno == EINTR) {
                // The select call was interrupted by a signal.
                // Restart the select call.
                continue;

            } else {
                perror("ERROR in select");
                exit(1);
            }
        }
        if (FD_ISSET(0, &readfds)) {
            // There is input available on stdin. Save in buffer.
            fgets(buffer, 1024, stdin);
            printf("Received input: %s", buffer);
            fflush(stdout);

            char *token;
            char *signal_name;
            int in_pid;
            int in_sig;

            // Tokenize the buffer string. (make it iterable with blocks of worlds.
            token = strtok(buffer, " ");

            // Skip the "kill" command. (That's what the NULL command in arg1 does.)
            token = strtok(NULL, " ");

            // Extract the signal name.
            signal_name = token + 1; // skip the "-" character.

            // Extract the PID.
            token = strtok(NULL, " ");
            // Use string complement(simpliroma) span function to remove \n from the end.
            token[strcspn(token, "\n")] = '\0';
            in_pid = atoi(token);

            printf("parent: signal is %s, and in_pid is %d\n", signal_name, in_pid);

            fflush(stdout);

            in_sig = sig_value(signal_name);

            int in_list[2] = {in_pid, in_sig};

            // PARENT ONLY CHECKS FOR HIS OWN PID
            if (in_list[0] == getpid()) printf("Parent found his pid in terminal!\n");
            else fwrite(buffer, sizeof(char), strlen(buffer), stdout);

        }
        sleep(1);
    }

    return 0;
}


int sig_value(char *sig_name) {
    /**
     * Returns the value of the signal name only if
     * it is either sigusr1 or 2 or sigterm.
     */
    if (strcmp(sig_name, "SIGUSR1") == 0) { return 10; }
    else if (strcmp(sig_name, "SIGUSR2") == 0) { return 12; }
    else if (strcmp(sig_name, "SIGTERM") == 0) { return 15; }
    else {
        perror("wrong signal name\n");
        return -1;
    }
}