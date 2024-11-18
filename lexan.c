#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


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

    int pipes[numOfSplitters][numOfBuilders][2];    // Pipe structure pipe[0] => read, pipe[1] => write

    for (int s = 0; s < numOfSplitters; s++) {  // Splitters
        for (int b = 0; b < numOfBuilders; b++) {   // Builders
            if (pipe(pipes[s][b]) == -1) {
                printf("Pipe creation failed\n");
                return 1;
            }
        }
    }
    
    //////// Create arrays to hold process ids of builders & splitters ////////
    int pidsBuilders[numOfBuilders];
    int pidsSplitter[numOfSplitters];

    //////// FORK SPLITTERS ////////

    for (int i = 0; i < numOfSplitters; i++){
        pidsSplitter[i] = fork();
        if (pidsSplitter[i] == -1){
            printf("Error with creating splitter process\n");
            return 2;
        }

        if (pidsSplitter[i] == 0){
            // Child process
            for (int s = 0; s < numOfSplitters; s++){
                for (int b = 0; b < numOfBuilders; b++) {
                    if (s == i){
                        close(pipes[s][b][0]); // Close read ends for this splitter
                        // Write ends (pipes[s][b][1]) remain open
                    } else {
                        close(pipes[s][b][0]); // Close all read ends for other splitters
                        close(pipes[s][b][1]); // Close all write ends for other splitters
                    }
                }
            }

            splitter(i, numOfSplitters, numOfBuilders, inputFile, 17, pipes);

            // Close all write ends after writing
            for (int b = 0; b < numOfBuilders; b++) {
                close(pipes[i][b][1]);
            }

            return 0;   // Since the process exits here, the for loop isn't executed by the child processes
            // break; if you want to continute after the for loop
        }

    }


    //////// FORK BUILDERS ////////

    for (int i = 0; i < numOfBuilders; i++){
        pidsBuilders[i] = fork();
        if (pidsBuilders[i] == -1){
            printf("Error with creating builder process\n");
            return 2;
        }

        if (pidsBuilders[i] == 0){
            // Child process
            // Close pipes
            for (int s = 0; s < numOfSplitters; s++){
                for (int b = 0; b < numOfBuilders; b++) {
                    if (s == i){
                        close(pipes[s][b][1]); // Close write ends for this splitter
                        // Read ends (pipes[s][b][0]) remain open
                    } else {
                        close(pipes[s][b][0]); // Close all read ends for other splitters
                        close(pipes[s][b][1]); // Close all write ends for other splitters
                    }
                }
            }

            builder(i, numOfSplitters, numOfBuilders, pipes);

            // Close all read ends after reading
            for (int s = 0; s < numOfSplitters; s++) {
                close(pipes[s][i][0]);
            }

            return 0;   // Since the process exits here, the for loop isn't executed by the child processes
            // break; if you want to continute after the for loop
        }
    }

    // PARENT PROCESS



    // Cloes all pipe ends
    for (int s = 0; s < numOfSplitters; s++) {
        for (int b = 0; b < numOfBuilders; b++) {
            close(pipes[s][b][0]);
            close(pipes[s][b][1]);
        }
    }

    // Wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }

    ////// PRINT COMMAND LINE ARGUMENTS ////////
    // printf("Input File: %s\n", inputFile);
    // printf("Number of Splitters: %d\n", numOfSplitters);
    // printf("Number of Builders: %d\n", numOfBuilders);
    // printf("Top-K Words: %d\n", topK);
    // printf("Exclusion List File: %s\n", exclusionList);
    // printf("Output File: %s\n", outputFile);

    return 0;
}