#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>

// todo gia elegxo sto an to pid einai swsto tha mporousame
//  na stelnoyme thn lista me ta pid sto cild meso toy execv
//  kai na elegxoyme an einai swsto. isos ginei me ton parent

int gate;
time_t end, start;
char state;


int sig_value(char *sig_name);


void sigalarm_handler(int sig) {

    end = time(NULL);
    double total_time = difftime(end, start);
    int final_time = (int) (total_time);
    printf("[Gate=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), final_time,
           (state == 'f') ? "closed" : "open");
    alarm(15);
}


void sigusr2_handler(int sig) {

    end = time(NULL);
    double total_time = difftime(end, start);
    int final_time = (int) (total_time);
    printf("[Gate=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), final_time,
           (state == 'f') ? "closed" : "open");
}

void sigusr1_handler(int sig) {
    state = (state == 't') ? 'f' : 't';
    end = time(NULL);
    double total_time = difftime(end, start);
    int final_time = (int) (total_time);
    printf("[Gate=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), final_time,
           (state == 'f') ? "closed" : "open");
}


int main(int argc, char **argv) {

    start = (time_t) strtoull(argv[3], NULL, 10);
    gate = atoi(argv[1]);
    state = argv[2][0];

    alarm(15);
    struct sigaction sa;
    sa.sa_handler = sigalarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    struct sigaction sa1;
    sa1.sa_handler = sigusr1_handler;
    sa1.sa_flags = 0;
    sigemptyset(&sa1.sa_mask);

    struct sigaction sa2;
    sa2.sa_handler = sigusr2_handler;
    sa2.sa_flags = 0;
    sigemptyset(&sa2.sa_mask);


    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGUSR1, &sa1, NULL);
    sigaction(SIGUSR2, &sa2, NULL);

    //printf("child process\n");
    printf("[Gate=%d/PID=%d/TIME=0s] The gates are %s!\n", gate, getpid(), (state == 'f') ? "closed" : "open");


    // reads from terminal and fills in_list with pid and signal
    fd_set readfds;
    char buffer[1024];


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

            printf("child: signal is %s, and in_pid is %d\n", signal_name, in_pid);

            fflush(stdout);

            in_sig = sig_value(signal_name);

            int in_list[2] = {in_pid, in_sig};


            if (in_list[1] == sig_value("SIGTERM"))
                printf("[PARENT/PID=%d] Child %d PID=%d exited\n", getppid(), gate, in_list[0]);

            kill(in_list[0], in_list[1]); // to kalei opoio paidi prolabei
//            else fwrite(initial_input, sizeof(char), strlen(initial_input), stdout);

            // todo sigterm to send signal to parent to make new child &
            //  make sigstop & sigcont signals
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
    else { perror("wrong signal name\n"); }
}