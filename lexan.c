#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include <arpa/inet.h> // For htonl and ntohl

#include "splitter.h"
#include "builder.h"

int main(int argc, char *argv[]) {

    //////// HANDLE COMMAND LINE ARGUMENTS ////////

    char *inputFile = NULL;
    int numOfSplitters = 0;
    int numOfBuilders = 0;
    int topK = 0;
    char *exclusionList = NULL;
    char *outputFile = NULL;

    // Parse the arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            numOfSplitters = atoi(argv[++i]);        // Number of Splitters initialized in the command line
        } else if (strcmp(argv[i], "-m") == 0) {
            numOfBuilders = atoi(argv[++i]);        // Number of Builders initialized in the command line
        } else if (strcmp(argv[i], "-t") == 0) {
            topK = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0) {
            exclusionList = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFile = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    //////// CREATE PIPES ////////
    // pipe[0] => read, 
    // pipe[1] => write

    
    int builderPipes[numOfBuilders][2]; // [][0] for reading by builders, [][1] for writing by splitters

    // Create one pipe per builder
    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(builderPipes[b]) == -1) {
            perror("Pipe creation failed");
            exit(1);
        }
    }

    //////// FORK SPLITTERS ////////

    for (int s = 0; s < numOfSplitters; s++) {
        int pid = fork();
        if (pid == -1) {
            perror("Error creating splitter process");
            return 2;
        }

        if (pid == 0) { // Child process (splitter)
            for(int b = 0;b < numOfBuilders; b++){
                close(builderPipes[b][0]);  // Close read ends of each builder pipe
            }

            // ADD LOGIC HERE


            for (int i = 3; i < 6; i++) {
                int builderIndex = i % numOfBuilders; // Determine which builder to send to

                // Convert integer to network byte order for consistent endianness
                int value = htonl(i);

                // Write the integer to the appropriate builder's pipe
                ssize_t n = write(builderPipes[builderIndex][1], &value, sizeof(value));
                if (n != sizeof(value)) {
                    perror("write");
                    exit(1);
                }

                printf("Splitter %d sent value %d to Builder %d\n", s, i, builderIndex);
            }

            // Close write ends before exiting
            for (int b = 0; b < numOfBuilders; b++) {
                close(builderPipes[b][1]); // Close write ends
            }


            // Exit after processing
            return 1;
        }
    }


    //////// FORK BUILDERS ////////
    for (int b = 0; b < numOfBuilders; b++) {
        int pid = fork();
        if (pid == -1) {
            perror("Error creating builder process");
            return 2;
        }

        if (pid == 0) { // Child process

            // Close unnecessary pipe ends
            // for (int i = 0; i < numOfBuilders; i++) {
            //     if (i != b) {
            //         close(builderPipes[i][0]); // Close read end of other builders
            //         close(builderPipes[i][1]); // Close write end of other builders
            //     }
            // }

            // close(builderPipes[b][1]);    // Close write end of own pipe


            // ADD LOGIC HERE

            for (int i = 0; i < numOfBuilders; i++) {
                close(builderPipes[i][1]); // Close write ends
                if (i != b) {
                    close(builderPipes[i][0]); // Close read ends of other builders
                }
            }

            // Read integers from the pipe
            int value;
            ssize_t n;
            while ((n = read(builderPipes[b][0], &value, sizeof(value))) > 0) {
                if (n != sizeof(value)) {
                    fprintf(stderr, "Partial read\n");
                    exit(1);
                }

                // Convert from network byte order to host byte order
                value = ntohl(value);

                printf("Builder %d received value %d\n", b, value);
            }

            if (n == -1) {
                perror("read");
                exit(1);
            }

            // Close read end before exiting
            close(builderPipes[b][0]);


            // Exit after processing
            return 1;
        }

    }


    // PARENT PROCESS

    // Cloes all pipe ends
    for (int b = 0; b < numOfBuilders; b++) {
        close(builderPipes[b][0]);
        close(builderPipes[b][1]);
    }
    

    // Wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }



    return 0;
}