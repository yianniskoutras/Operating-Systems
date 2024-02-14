#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>


#define MAX_BUFFER_SIZE 1024


int main(int argc, char **argv) {

 int debug=0;

    if(argc!=1 && argc!=2 && argc!=3 && argc!=4 && argc!=5 && argc!=6){
        perror("./Usage: [--host HOST]/[--port PORT]/[--debug]\n");
        exit(EXIT_FAILURE);
    }


    // Define the host and port information

    char* host = "iot.dslab.pub.ds.open-cloud.xyz";
    int port = 18080;



    int dummy = 0;

    for(int i=1; i<argc; i++){

        if(dummy){
            dummy = 0;
            continue;
        }

        if(strcmp("--debug", argv[i]) == 0) {
            debug = 1;
            continue;
        }

        if(strcmp("--host", argv[i]) == 0) {
            host = argv[i+1];
            dummy = 1 ;
            continue;
        }

        if(strcmp("--port", argv[i]) == 0) {
            port = atoi(argv[i+1]);
            if(port == 0) {
                perror("/Usage: [--host HOST]/[--port PORT]/[--debug]\n");
                exit(EXIT_FAILURE);
            }
            dummy = 1;
            continue;
        }

        perror("/Usage: [--host HOST]/[--port PORT]/[--debug]\n");
        exit(EXIT_FAILURE);

    }




     // Create a socket to communicate with host

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }


    // Retrieve host information

    struct hostent* host_info = gethostbyname(host);
    if (host_info == NULL) {
        perror("Failed to retrieve host information");
        exit(EXIT_FAILURE);
    }


    // Set up the server address


    //from here
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr, host_info->h_addr, host_info->h_length);
    //to here   read sockaddr_in


    // Connect to the server

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }



    // Connection successful: we can now send/receive data from host

    char buffer[MAX_BUFFER_SIZE];
    int flag;

    while(1) {
        char input[MAX_BUFFER_SIZE];

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "help") == 0) {
            printf("./Usage: [get]/[N name surname reason]/[exit]\n");
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            printf("Program will now terminate\n");
            close(client_socket);   //Close socket before exiting
            return 0;
        }

        flag = (strcmp(input, "get") == 0) ? 0 : 1;


        // Send data to the server

        if (write(client_socket, input, strlen(input)) == -1) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }

        if(debug) printf("[--DEBUG] Sent '%s'\n", input);    //Print the message I have just sent


        // Receive data from the server

        ssize_t received_bytes = read(client_socket, buffer, sizeof(buffer));
        if (received_bytes == -1) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }


        if (debug) printf("[--DEBUG] Read %s", buffer);


        if(!flag) {

            printf("---------------------------\n");
            int index = atoi(&buffer[0]);

            char event[10];

            switch (index) {
                case 0:
                    strcpy(event, "boot");
                    event[5] = '\0';
                    break;

                case 1:
                    strcpy(event, "setup");
                    event[5] = '\0';
                    break;

                case 2:
                    strcpy(event, "interval");
                    event[8] = '\0';
                    break;

                case 3:
                    strcpy(event, "button");
                    event[6] = '\0';
                    break;

                case 4:
                    strcpy(event, "motion");
                    event[6] = '\0';
                    break;
            }

            char brightness[3], temperature[5], t_stamp[11];
            double final_temperature;
            int final_brightness;
            time_t timestamp;

            for (int i = 0; i < 2; i++) {
                brightness[i] = buffer[i + 3];
            }

            brightness[2] = '\0';

            for (int i = 0; i < 4; i++) {
                temperature[i] = buffer[i + 6];
            }

            temperature[4] = '\0';

            for (int i = 0; i < 10; i++) {
                t_stamp[i] = buffer[i + 11];
            }

            t_stamp[10] = '\0';


            sscanf(temperature, "%lf", &final_temperature);
            final_temperature /= 100;

            final_brightness = atoi(brightness);

            timestamp = atoi(t_stamp);

            struct tm* local_time = localtime(&timestamp);
            if (local_time == NULL) {
                perror("Failed to convert timestamp to local time");
                exit(EXIT_FAILURE);
            }

            // Retrieve wanted date

            int year = local_time->tm_year + 1900;
            int mon = local_time->tm_mon + 1;
            int day = local_time->tm_mday;
            int hour = local_time->tm_hour;
            int min = local_time->tm_min;
            int sec = local_time->tm_sec;


            // Print the received data

            printf("Latest event:\n%s (%d)\nTemperature is: %.2lf\nLight level is: %d\n", event, index, final_temperature, final_brightness);
            printf("Timestamp is: %04d-%02d-%02d %02d:%02d:%02d\n", year, mon, day, hour, min, sec);

        }
            else {

            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';

            if (debug) printf("[--DEBUG] Sent '%s'\n", input);  // I have sent the verification code

            if (write(client_socket, input, strlen(input)) == -1) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }

            received_bytes = read(client_socket, buffer, sizeof(buffer));  // I received acknowledgment

            if (received_bytes == -1) {
                perror("Receive failed");
                exit(EXIT_FAILURE);
            }

            if (debug) printf("[--DEBUG] Read %s", buffer);     // Print acknowledgment

        }
    }


    return 0;
}
